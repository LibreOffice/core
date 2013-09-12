/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <set>
#include <stack>
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/container/ElementExistException.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/reflection/InvalidTypeNameException.hpp"
#include "com/sun/star/reflection/NoSuchTypeNameException.hpp"
#include "com/sun/star/reflection/TypeDescriptionSearchDepth.hpp"
#include "com/sun/star/reflection/XConstantTypeDescription.hpp"
#include "com/sun/star/reflection/XConstantsTypeDescription.hpp"
#include "com/sun/star/reflection/XEnumTypeDescription.hpp"
#include "com/sun/star/reflection/XIndirectTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceAttributeTypeDescription2.hpp"
#include "com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceTypeDescription2.hpp"
#include "com/sun/star/reflection/XModuleTypeDescription.hpp"
#include "com/sun/star/reflection/XPublished.hpp"
#include "com/sun/star/reflection/XServiceTypeDescription2.hpp"
#include "com/sun/star/reflection/XSingletonTypeDescription2.hpp"
#include "com/sun/star/reflection/XStructTypeDescription.hpp"
#include "com/sun/star/reflection/XTypeDescription.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "cppu/unotype.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/supportsservice.hxx"
#include "osl/file.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/ustring.hxx"
#include "sal/macros.h"
#include "sal/types.h"

using rtl::OUString;

#include "unoidl/unoidl.hxx"

#include "paths.hxx"
#include "typemanager.hxx"

namespace {

rtl::OUString makePrefix(rtl::OUString const & name) {
    return name.isEmpty() ? name : name + ".";
}

css::uno::Any resolveTypedefs(css::uno::Any const & type) {
    for (css::uno::Any t(type);;) {
        css::uno::Reference< css::reflection::XIndirectTypeDescription > ind(
            type, css::uno::UNO_QUERY);
        if (!ind.is() || ind->getTypeClass() != css::uno::TypeClass_TYPEDEF) {
            return t;
        }
        t = css::uno::makeAny(ind->getReferencedType());
    }
}

class SimpleTypeDescription:
    public cppu::WeakImplHelper1< css::reflection::XTypeDescription >
{
public:
    SimpleTypeDescription(
        css::uno::TypeClass typeClass, rtl::OUString const & name):
        typeClass_(typeClass), name_(name)
    {}

private:
    virtual ~SimpleTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return typeClass_; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    css::uno::TypeClass typeClass_;
    rtl::OUString name_;
};

class SequenceTypeDescription:
    public cppu::WeakImplHelper1< css::reflection::XIndirectTypeDescription >
{
public:
    SequenceTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name, rtl::OUString const & componentType):
        manager_(manager), name_(name), componentType_(componentType)
    { assert(manager.is()); }

private:
    virtual ~SequenceTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_SEQUENCE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getReferencedType() throw (css::uno::RuntimeException)
    { return manager_->resolve(componentType_); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::OUString componentType_;
};

class PublishableDescription:
    public cppu::WeakImplHelper1< css::reflection::XPublished >
{
protected:
    PublishableDescription(bool published): published_(published) {}

    virtual ~PublishableDescription() {}

private:
    virtual sal_Bool SAL_CALL isPublished() throw (css::uno::RuntimeException)
    { return published_; }

    bool published_;
};

class ModuleDescription:
    public cppu::WeakImplHelper1< css::reflection::XModuleTypeDescription >
{
public:
    ModuleDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::ModuleEntity > const & entity):
        manager_(manager), name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~ModuleDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_MODULE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMembers() throw (css::uno::RuntimeException);

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::ModuleEntity > entity_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
ModuleDescription::getMembers() throw (css::uno::RuntimeException) {
    try {
        std::vector< rtl::OUString > names(entity_->getMemberNames());
        assert(names.size() <= SAL_MAX_INT32);
        sal_Int32 n = static_cast< sal_Int32 >(names.size());
        css::uno::Sequence<
            css::uno::Reference< css::reflection::XTypeDescription > > s(n);
        for (sal_Int32 i = 0; i != n; ++i) {
            s[i] = manager_->resolve(makePrefix(name_) + names[i]);
        }
        return s;
    } catch (unoidl::FileFormatException & e) {
        throw css::uno::DeploymentException(
            e.getUri() + ": " + e.getDetail(),
            static_cast< cppu::OWeakObject * >(this));
    }
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XEnumTypeDescription >
EnumTypeDescription_Base;

class EnumTypeDescription: public EnumTypeDescription_Base {
public:
    EnumTypeDescription(
        rtl::OUString const & name,
        rtl::Reference< unoidl::EnumTypeEntity > const & entity):
        EnumTypeDescription_Base(entity->isPublished()), name_(name),
        entity_(entity)
    { assert(entity.is()); }

private:
    virtual ~EnumTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_ENUM; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual sal_Int32 SAL_CALL getDefaultEnumValue()
        throw (css::uno::RuntimeException)
    { return entity_->getMembers()[0].value; }

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getEnumNames()
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getEnumValues()
        throw (css::uno::RuntimeException);

    rtl::OUString name_;
    rtl::Reference< unoidl::EnumTypeEntity > entity_;
};

css::uno::Sequence< rtl::OUString > EnumTypeDescription::getEnumNames()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = entity_->getMembers()[i].name;
    }
    return s;
}

css::uno::Sequence< sal_Int32 > EnumTypeDescription::getEnumValues()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence< sal_Int32 > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = entity_->getMembers()[i].value;
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XStructTypeDescription >
PlainStructTypeDescription_Base;

class PlainStructTypeDescription: public PlainStructTypeDescription_Base {
public:
    PlainStructTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::PlainStructTypeEntity > const & entity):
        PlainStructTypeDescription_Base(entity->isPublished()),
        manager_(manager), name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~PlainStructTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_STRUCT; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() throw (css::uno::RuntimeException) {
        return entity_->getDirectBase().isEmpty()
            ? css::uno::Reference< css::reflection::XTypeDescription >()
            : manager_->resolve(entity_->getDirectBase());
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getMemberNames()
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getTypeParameters()
        throw (css::uno::RuntimeException)
    { return css::uno::Sequence< rtl::OUString >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getTypeArguments() throw (css::uno::RuntimeException) {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XTypeDescription > >();
    }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::PlainStructTypeEntity > entity_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
PlainStructTypeDescription::getMemberTypes() throw (css::uno::RuntimeException)
{
    assert(entity_->getDirectMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getDirectMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = manager_->resolve(entity_->getDirectMembers()[i].type);
    }
    return s;
}

css::uno::Sequence< rtl::OUString > PlainStructTypeDescription::getMemberNames()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getDirectMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getDirectMembers().size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = entity_->getDirectMembers()[i].name;
    }
    return s;
}

class ParameterizedMemberTypeDescription:
    public cppu::WeakImplHelper1< css::reflection::XTypeDescription >
{
public:
    explicit ParameterizedMemberTypeDescription(
        rtl::OUString const & typeParameterName):
        typeParameterName_(typeParameterName)
    {}

private:
    virtual ~ParameterizedMemberTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_UNKNOWN; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return typeParameterName_; }

    rtl::OUString typeParameterName_;
};

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XStructTypeDescription >
PolymorphicStructTypeTemplateDescription_Base;

class PolymorphicStructTypeTemplateDescription:
    public PolymorphicStructTypeTemplateDescription_Base
{
public:
    PolymorphicStructTypeTemplateDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > const &
            entity):
        PolymorphicStructTypeTemplateDescription_Base(entity->isPublished()),
        manager_(manager), name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~PolymorphicStructTypeTemplateDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_STRUCT; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() throw (css::uno::RuntimeException)
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getMemberNames()
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getTypeParameters()
        throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getTypeArguments() throw (css::uno::RuntimeException) {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XTypeDescription > >();
    }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > entity_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
PolymorphicStructTypeTemplateDescription::getMemberTypes()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = entity_->getMembers()[i].parameterized
            ? new ParameterizedMemberTypeDescription(
                entity_->getMembers()[i].type)
            : manager_->resolve(entity_->getMembers()[i].type);
    }
    return s;
}

css::uno::Sequence< rtl::OUString >
PolymorphicStructTypeTemplateDescription::getMemberNames()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = entity_->getMembers()[i].name;
    }
    return s;
}

css::uno::Sequence< rtl::OUString >
PolymorphicStructTypeTemplateDescription::getTypeParameters()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getTypeParameters().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getTypeParameters().size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = entity_->getTypeParameters()[i];
    }
    return s;
}

class InstantiatedPolymorphicStructTypeDescription:
    public cppu::WeakImplHelper1< css::reflection::XStructTypeDescription >
{
public:
    InstantiatedPolymorphicStructTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > const &
            entity,
        std::vector< rtl::OUString > const & arguments):
        manager_(manager), name_(name), entity_(entity), arguments_(arguments)
    {
        assert(manager.is());
        assert(entity.is());
        assert(arguments.size() == entity->getTypeParameters().size());
    }

private:
    virtual ~InstantiatedPolymorphicStructTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_STRUCT; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() throw (css::uno::RuntimeException)
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getMemberNames()
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getTypeParameters()
        throw (css::uno::RuntimeException)
    { return css::uno::Sequence< rtl::OUString >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getTypeArguments() throw (css::uno::RuntimeException);

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > entity_;
    std::vector< rtl::OUString > arguments_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InstantiatedPolymorphicStructTypeDescription::getMemberTypes()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        rtl::OUString type(entity_->getMembers()[i].type);
        if (entity_->getMembers()[i].parameterized) {
            for (std::vector< rtl::OUString >::const_iterator j(
                     entity_->getTypeParameters().begin());
                 j != entity_->getTypeParameters().end(); ++j)
            {
                if (*j == type) {
                    type = arguments_[j - entity_->getTypeParameters().begin()];
                    goto found;
                }
            }
            assert(false); // this cannot happen                         //TODO!
        found:;
        }
        s[i] = manager_->resolve(type);
    }
    return s;
}

css::uno::Sequence< rtl::OUString >
InstantiatedPolymorphicStructTypeDescription::getMemberNames()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = entity_->getMembers()[i].name;
    }
    return s;
}
css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InstantiatedPolymorphicStructTypeDescription::getTypeArguments()
    throw (css::uno::RuntimeException)
{
    assert(arguments_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(arguments_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = manager_->resolve(arguments_[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XCompoundTypeDescription >
ExceptionTypeDescription_Base;

class ExceptionTypeDescription: public ExceptionTypeDescription_Base {
public:
    ExceptionTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::ExceptionTypeEntity > const & entity):
        ExceptionTypeDescription_Base(entity->isPublished()), manager_(manager),
        name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~ExceptionTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_EXCEPTION; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() throw (css::uno::RuntimeException) {
        return entity_->getDirectBase().isEmpty()
            ? css::uno::Reference< css::reflection::XTypeDescription >()
            : manager_->resolve(entity_->getDirectBase());
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getMemberNames()
        throw (css::uno::RuntimeException);

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::ExceptionTypeEntity > entity_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
ExceptionTypeDescription::getMemberTypes() throw (css::uno::RuntimeException) {
    assert(entity_->getDirectMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getDirectMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = manager_->resolve(entity_->getDirectMembers()[i].type);
    }
    return s;
}

css::uno::Sequence< rtl::OUString > ExceptionTypeDescription::getMemberNames()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getDirectMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getDirectMembers().size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = entity_->getDirectMembers()[i].name;
    }
    return s;
}

class AttributeDescription:
    public cppu::WeakImplHelper1<
        css::reflection::XInterfaceAttributeTypeDescription2 >
{
public:
    AttributeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        unoidl::InterfaceTypeEntity::Attribute const & attribute,
        sal_Int32 position):
        manager_(manager), name_(name), attribute_(attribute),
        position_(position)
    { assert(manager.is()); }

private:
    virtual ~AttributeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_INTERFACE_ATTRIBUTE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual rtl::OUString SAL_CALL getMemberName()
        throw (css::uno::RuntimeException)
    { return attribute_.name; }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return position_; }

    virtual sal_Bool SAL_CALL isReadOnly() throw (css::uno::RuntimeException)
    { return attribute_.readOnly; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() throw (css::uno::RuntimeException)
    { return manager_->resolve(attribute_.type); }

    virtual sal_Bool SAL_CALL isBound() throw (css::uno::RuntimeException)
    { return attribute_.bound; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
    SAL_CALL getGetExceptions() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
    SAL_CALL getSetExceptions() throw (css::uno::RuntimeException);

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    unoidl::InterfaceTypeEntity::Attribute attribute_;
    sal_Int32 position_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
AttributeDescription::getGetExceptions() throw (css::uno::RuntimeException) {
    assert(attribute_.getExceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(attribute_.getExceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            manager_->resolve(attribute_.getExceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
AttributeDescription::getSetExceptions() throw (css::uno::RuntimeException) {
    assert(attribute_.setExceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(attribute_.setExceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            manager_->resolve(attribute_.setExceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

class MethodParameter:
    public cppu::WeakImplHelper1< css::reflection::XMethodParameter >
{
public:
    MethodParameter(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        unoidl::InterfaceTypeEntity::Method::Parameter const & parameter,
        sal_Int32 position):
        manager_(manager), parameter_(parameter), position_(position)
    { assert(manager.is()); }

private:
    virtual ~MethodParameter() {}

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return parameter_.name; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() throw (css::uno::RuntimeException)
    { return manager_->resolve(parameter_.type); }

    virtual sal_Bool SAL_CALL isIn() throw (css::uno::RuntimeException) {
        return
            (parameter_.direction
             == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
            || (parameter_.direction
                == unoidl::InterfaceTypeEntity::Method::Parameter::
                    DIRECTION_IN_OUT);
    }

    virtual sal_Bool SAL_CALL isOut() throw (css::uno::RuntimeException) {
        return
            (parameter_.direction
             == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_OUT)
            || (parameter_.direction
                == unoidl::InterfaceTypeEntity::Method::Parameter::
                    DIRECTION_IN_OUT);
    }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return position_; }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    unoidl::InterfaceTypeEntity::Method::Parameter parameter_;
    sal_Int32 position_;
};

class MethodDescription:
    public cppu::WeakImplHelper1<
        css::reflection::XInterfaceMethodTypeDescription >
{
public:
    MethodDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        unoidl::InterfaceTypeEntity::Method const & method, sal_Int32 position):
        manager_(manager), name_(name), method_(method), position_(position)
    { assert(manager.is()); }

private:
    virtual ~MethodDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_INTERFACE_METHOD; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual rtl::OUString SAL_CALL getMemberName()
        throw (css::uno::RuntimeException)
    { return method_.name; }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return position_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getReturnType() throw (css::uno::RuntimeException)
    { return manager_->resolve(method_.returnType); }

    virtual sal_Bool SAL_CALL isOneway() throw (css::uno::RuntimeException)
    { return false; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XMethodParameter > >
    SAL_CALL getParameters() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getExceptions() throw (css::uno::RuntimeException);

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    unoidl::InterfaceTypeEntity::Method method_;
    sal_Int32 position_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XMethodParameter > >
MethodDescription::getParameters() throw (css::uno::RuntimeException) {
    assert(method_.parameters.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(method_.parameters.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XMethodParameter > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = new MethodParameter(manager_, method_.parameters[i], i);
    }
    return s;
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
MethodDescription::getExceptions() throw (css::uno::RuntimeException) {
    assert(method_.exceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(method_.exceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = manager_->resolve(method_.exceptions[i]);
    }
    return s;
}

class BaseOffset: private boost::noncopyable {
public:
    BaseOffset(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
            const & description);

    sal_Int32 get() const { return offset_; }

private:
    void calculateBases(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
            const & description);

    void calculate(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
            const & description);

    std::set< rtl::OUString > set_;
    sal_Int32 offset_;
};

BaseOffset::BaseOffset(
    css::uno::Reference< css::reflection::XInterfaceTypeDescription2 > const &
        description):
    offset_(0)
{
    calculateBases(description);
}

void BaseOffset::calculateBases(
    css::uno::Reference< css::reflection::XInterfaceTypeDescription2 > const &
        description)
{
    css::uno::Sequence<
        css::uno::Reference < css::reflection::XTypeDescription > > bases(
            description->getBaseTypes());
    for (sal_Int32 i = 0; i != bases.getLength(); ++i) {
        calculate(
            css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >(
                resolveTypedefs(css::uno::makeAny(bases[i])),
                css::uno::UNO_QUERY_THROW));
    }
}

void BaseOffset::calculate(
    css::uno::Reference< css::reflection::XInterfaceTypeDescription2 > const &
        description)
{
    if (set_.insert(description->getName()).second) {
        calculateBases(description);
        offset_ += description->getMembers().getLength();
    }
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XInterfaceTypeDescription2 >
InterfaceTypeDescription_Base;

class InterfaceTypeDescription: public InterfaceTypeDescription_Base {
public:
    InterfaceTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::InterfaceTypeEntity > const & entity):
        InterfaceTypeDescription_Base(entity->isPublished()), manager_(manager),
        name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~InterfaceTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_INTERFACE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() throw (css::uno::RuntimeException) {
        return entity_->getDirectMandatoryBases().empty()
            ? css::uno::Reference< css::reflection::XTypeDescription >()
            : manager_->resolve(entity_->getDirectMandatoryBases()[0].name);
    }

    virtual css::uno::Uik SAL_CALL getUik() throw (css::uno::RuntimeException)
    { return css::uno::Uik(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference<
             css::reflection::XInterfaceMemberTypeDescription > >
    SAL_CALL getMembers() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getBaseTypes() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getOptionalBaseTypes() throw (css::uno::RuntimeException);

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::InterfaceTypeEntity > entity_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XInterfaceMemberTypeDescription > >
InterfaceTypeDescription::getMembers() throw (css::uno::RuntimeException) {
    assert(
        entity_->getDirectAttributes().size() <= SAL_MAX_INT32
        && (entity_->getDirectMethods().size()
            <= SAL_MAX_INT32 - entity_->getDirectAttributes().size()));
    sal_Int32 n1 = static_cast< sal_Int32 >(
        entity_->getDirectAttributes().size());
    sal_Int32 n2 = static_cast< sal_Int32 >(entity_->getDirectMethods().size());
    css::uno::Sequence<
        css::uno::Reference<
            css::reflection::XInterfaceMemberTypeDescription > > s(n1 + n2);
    sal_Int32 off = BaseOffset(this).get();
    for (sal_Int32 i = 0; i != n1; ++i) {
        s[i] = new AttributeDescription(
            manager_, name_ + "::" + entity_->getDirectAttributes()[i].name,
            entity_->getDirectAttributes()[i], off + i);
    }
    for (sal_Int32 i = 0; i != n2; ++i) {
        s[n1 + i] = new MethodDescription(
            manager_, name_ + "::" + entity_->getDirectMethods()[i].name,
            entity_->getDirectMethods()[i], off + n1 + i);
    }
    return s;
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InterfaceTypeDescription::getBaseTypes() throw (css::uno::RuntimeException) {
    assert(entity_->getDirectMandatoryBases().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectMandatoryBases().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = manager_->resolve(entity_->getDirectMandatoryBases()[i].name);
    }
    return s;
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InterfaceTypeDescription::getOptionalBaseTypes()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getDirectOptionalBases().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectOptionalBases().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = manager_->resolve(entity_->getDirectOptionalBases()[i].name);
    }
    return s;
}

class ConstantDescription:
    public cppu::WeakImplHelper1< css::reflection::XConstantTypeDescription >
{
public:
    ConstantDescription(
        rtl::OUString const & constantGroupName,
        unoidl::ConstantGroupEntity::Member const & member);

private:
    virtual ~ConstantDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_CONSTANT; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Any SAL_CALL getConstantValue()
        throw (css::uno::RuntimeException)
    { return value_; }

    rtl::OUString name_;
    css::uno::Any value_;
};

ConstantDescription::ConstantDescription(
    rtl::OUString const & constantGroupName,
    unoidl::ConstantGroupEntity::Member const & member):
    name_(makePrefix(constantGroupName) + member.name)
{
    switch (member.value.type) {
    case unoidl::ConstantValue::TYPE_BOOLEAN:
        value_ <<= member.value.booleanValue;
        break;
    case unoidl::ConstantValue::TYPE_BYTE:
        value_ <<= member.value.byteValue;
        break;
    case unoidl::ConstantValue::TYPE_SHORT:
        value_ <<= member.value.shortValue;
        break;
    case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
        value_ <<= member.value.unsignedShortValue;
        break;
    case unoidl::ConstantValue::TYPE_LONG:
        value_ <<= member.value.longValue;
        break;
    case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
        value_ <<= member.value.unsignedLongValue;
        break;
    case unoidl::ConstantValue::TYPE_HYPER:
        value_ <<= member.value.hyperValue;
        break;
    case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
        value_ <<= member.value.unsignedHyperValue;
        break;
    case unoidl::ConstantValue::TYPE_FLOAT:
        value_ <<= member.value.floatValue;
        break;
    case unoidl::ConstantValue::TYPE_DOUBLE:
        value_ <<= member.value.doubleValue;
        break;
    default:
        for (;;) { std::abort(); } // this cannot happen
    }
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XConstantsTypeDescription >
ConstantGroupDescription_Base;

class ConstantGroupDescription: public ConstantGroupDescription_Base {
public:
    ConstantGroupDescription(
        rtl::OUString const & name,
        rtl::Reference< unoidl::ConstantGroupEntity > const & entity):
        ConstantGroupDescription_Base(entity->isPublished()), name_(name),
        entity_(entity)
    { assert(entity.is()); }

private:
    virtual ~ConstantGroupDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_CONSTANTS; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XConstantTypeDescription > >
    SAL_CALL getConstants() throw (css::uno::RuntimeException);

    rtl::OUString name_;
    rtl::Reference< unoidl::ConstantGroupEntity > entity_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XConstantTypeDescription > >
ConstantGroupDescription::getConstants() throw (css::uno::RuntimeException) {
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XConstantTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = new ConstantDescription(name_, entity_->getMembers()[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XIndirectTypeDescription >
TypedefDescription_Base;

class TypedefDescription: public TypedefDescription_Base {
public:
    TypedefDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::TypedefEntity > const & entity):
        TypedefDescription_Base(entity->isPublished()), manager_(manager),
        name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~TypedefDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_TYPEDEF; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getReferencedType() throw (css::uno::RuntimeException)
    { return manager_->resolve(entity_->getType()); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::TypedefEntity > entity_;
};

class ConstructorParameter:
    public cppu::WeakImplHelper1< css::reflection::XParameter >
{
public:
    ConstructorParameter(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter
            const & parameter,
        sal_Int32 position):
        manager_(manager), parameter_(parameter), position_(position)
    { assert(manager.is()); }

private:
    virtual ~ConstructorParameter() {}

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return parameter_.name; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() throw (css::uno::RuntimeException)
    { return manager_->resolve(parameter_.type); }

    virtual sal_Bool SAL_CALL isIn() throw (css::uno::RuntimeException)
    { return true; }

    virtual sal_Bool SAL_CALL isOut() throw (css::uno::RuntimeException)
    { return false; }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return position_; }

    virtual sal_Bool SAL_CALL isRestParameter()
        throw (css::uno::RuntimeException)
    { return parameter_.rest; }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter
        parameter_;
    sal_Int32 position_;
};

class ConstructorDescription:
    public cppu::WeakImplHelper1<
        css::reflection::XServiceConstructorDescription >
{
public:
    ConstructorDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        unoidl::SingleInterfaceBasedServiceEntity::Constructor const &
            constructor):
        manager_(manager), constructor_(constructor)
    { assert(manager.is()); }

private:
    virtual ~ConstructorDescription() {}

    virtual sal_Bool SAL_CALL isDefaultConstructor()
        throw (css::uno::RuntimeException)
    { return constructor_.defaultConstructor; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return constructor_.name; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XParameter > >
    SAL_CALL getParameters() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
    SAL_CALL getExceptions() throw (css::uno::RuntimeException);

    rtl::Reference< cppuhelper::TypeManager > manager_;
    unoidl::SingleInterfaceBasedServiceEntity::Constructor constructor_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XParameter > >
ConstructorDescription::getParameters() throw (css::uno::RuntimeException) {
    assert(constructor_.parameters.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(constructor_.parameters.size());
    css::uno::Sequence< css::uno::Reference< css::reflection::XParameter > > s(
        n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = new ConstructorParameter(
            manager_, constructor_.parameters[i], i);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
ConstructorDescription::getExceptions() throw (css::uno::RuntimeException) {
    assert(constructor_.exceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(constructor_.exceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            manager_->resolve(constructor_.exceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XServiceTypeDescription2 >
SingleInterfaceBasedServiceDescription_Base;

class SingleInterfaceBasedServiceDescription:
    public SingleInterfaceBasedServiceDescription_Base
{
public:
    SingleInterfaceBasedServiceDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > const &
            entity):
        SingleInterfaceBasedServiceDescription_Base(entity->isPublished()),
        manager_(manager), name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~SingleInterfaceBasedServiceDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_SERVICE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getMandatoryServices() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XServiceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getOptionalServices() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XServiceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getMandatoryInterfaces() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XInterfaceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getOptionalInterfaces() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XInterfaceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XPropertyTypeDescription > >
    SAL_CALL getProperties() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XPropertyTypeDescription > >();
    }

    virtual sal_Bool SAL_CALL isSingleInterfaceBased()
        throw (css::uno::RuntimeException)
    { return true; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getInterface() throw (css::uno::RuntimeException)
    { return manager_->resolve(entity_->getBase()); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
    SAL_CALL getConstructors() throw (css::uno::RuntimeException);

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > entity_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XServiceConstructorDescription > >
SingleInterfaceBasedServiceDescription::getConstructors()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getConstructors().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getConstructors().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
            s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = new ConstructorDescription(
            manager_, entity_->getConstructors()[i]);
    }
    return s;
}

class PropertyDescription:
    public cppu::WeakImplHelper1< css::reflection::XPropertyTypeDescription >
{
public:
    PropertyDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        unoidl::AccumulationBasedServiceEntity::Property const & property):
        manager_(manager), property_(property)
    { assert(manager.is()); }

private:
    virtual ~PropertyDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_PROPERTY; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return property_.name; }

    virtual sal_Int16 SAL_CALL getPropertyFlags()
        throw (css::uno::RuntimeException)
    { return property_.attributes; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getPropertyTypeDescription() throw (css::uno::RuntimeException)
    { return manager_->resolve(property_.type); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    unoidl::AccumulationBasedServiceEntity::Property property_;
};

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XServiceTypeDescription2 >
AccumulationBasedServiceDescription_Base;

class AccumulationBasedServiceDescription:
    public AccumulationBasedServiceDescription_Base
{
public:
    AccumulationBasedServiceDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::AccumulationBasedServiceEntity > const &
            entity):
        AccumulationBasedServiceDescription_Base(entity->isPublished()),
        manager_(manager), name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~AccumulationBasedServiceDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_SERVICE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getMandatoryServices() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getOptionalServices() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getMandatoryInterfaces() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getOptionalInterfaces() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XPropertyTypeDescription > >
    SAL_CALL getProperties() throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isSingleInterfaceBased()
        throw (css::uno::RuntimeException)
    { return false; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getInterface() throw (css::uno::RuntimeException)
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
    SAL_CALL getConstructors() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XServiceConstructorDescription > >();
    }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::AccumulationBasedServiceEntity > entity_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XServiceTypeDescription > >
AccumulationBasedServiceDescription::getMandatoryServices()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getDirectMandatoryBaseServices().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectMandatoryBaseServices().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            manager_->resolve(
                entity_->getDirectMandatoryBaseServices()[i].name),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XServiceTypeDescription > >
AccumulationBasedServiceDescription::getOptionalServices()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getDirectOptionalBaseServices().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectOptionalBaseServices().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            manager_->resolve(entity_->getDirectOptionalBaseServices()[i].name),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
AccumulationBasedServiceDescription::getMandatoryInterfaces()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getDirectMandatoryBaseInterfaces().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectMandatoryBaseInterfaces().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > > s(
            n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolveTypedefs(
                manager_->find(
                    entity_->getDirectMandatoryBaseInterfaces()[i].name)),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
AccumulationBasedServiceDescription::getOptionalInterfaces()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getDirectOptionalBaseInterfaces().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectOptionalBaseInterfaces().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > > s(
            n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolveTypedefs(
                manager_->find(
                    entity_->getDirectOptionalBaseInterfaces()[i].name)),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XPropertyTypeDescription > >
AccumulationBasedServiceDescription::getProperties()
    throw (css::uno::RuntimeException)
{
    assert(entity_->getDirectProperties().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectProperties().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XPropertyTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = new PropertyDescription(
            manager_, entity_->getDirectProperties()[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XSingletonTypeDescription2 >
InterfaceBasedSingletonDescription_Base;

class InterfaceBasedSingletonDescription:
    public InterfaceBasedSingletonDescription_Base
{
public:
    InterfaceBasedSingletonDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::InterfaceBasedSingletonEntity > const & entity):
        InterfaceBasedSingletonDescription_Base(entity->isPublished()),
        manager_(manager), name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~InterfaceBasedSingletonDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_SINGLETON; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XServiceTypeDescription >
    SAL_CALL getService() throw (css::uno::RuntimeException)
    {
        return
            css::uno::Reference< css::reflection::XServiceTypeDescription >();
    }

    virtual sal_Bool SAL_CALL isInterfaceBased()
        throw (css::uno::RuntimeException)
    { return true; }

    virtual css::uno::Reference< css::reflection::XTypeDescription >
    SAL_CALL getInterface() throw (css::uno::RuntimeException)
    { return manager_->resolve(entity_->getBase()); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::InterfaceBasedSingletonEntity > entity_;
};

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XSingletonTypeDescription2 >
ServiceBasedSingletonDescription_Base;

class ServiceBasedSingletonDescription:
    public ServiceBasedSingletonDescription_Base
{
public:
    ServiceBasedSingletonDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & name,
        rtl::Reference< unoidl::ServiceBasedSingletonEntity > const & entity):
        ServiceBasedSingletonDescription_Base(entity_->isPublished()),
        manager_(manager), name_(name), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~ServiceBasedSingletonDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_SINGLETON; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XServiceTypeDescription >
    SAL_CALL getService() throw (css::uno::RuntimeException)
    {
        return css::uno::Reference< css::reflection::XServiceTypeDescription >(
            manager_->resolve(entity_->getBase()), css::uno::UNO_QUERY_THROW);
    }

    virtual sal_Bool SAL_CALL isInterfaceBased()
        throw (css::uno::RuntimeException)
    { return false; }

    virtual css::uno::Reference< css::reflection::XTypeDescription >
    SAL_CALL getInterface() throw (css::uno::RuntimeException)
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    rtl::OUString name_;
    rtl::Reference< unoidl::ServiceBasedSingletonEntity > entity_;
};

class Enumeration:
    public cppu::WeakImplHelper1< css::reflection::XTypeDescriptionEnumeration >
{
public:
    Enumeration(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        rtl::OUString const & prefix,
        rtl::Reference< unoidl::MapCursor > const & cursor,
        css::uno::Sequence< css::uno::TypeClass > const & types, bool deep):
        manager_(manager), types_(types), deep_(deep)
    {
        assert(manager.is());
        positions_.push(Position(prefix, cursor));
        findNextMatch();
    }

private:
    virtual ~Enumeration() {}

    virtual sal_Bool SAL_CALL hasMoreElements()
        throw (css::uno::RuntimeException)
    { return !positions_.empty(); }

    virtual css::uno::Any SAL_CALL nextElement()
        throw (
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException, css::uno::RuntimeException)
    { return css::uno::makeAny(nextTypeDescription()); }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    nextTypeDescription()
        throw (
            css::container::NoSuchElementException, css::uno::RuntimeException);

    bool matches(css::uno::TypeClass tc) const;

    void findNextMatch();

    struct Position {
        Position(
            rtl::OUString const & thePrefix,
            rtl::Reference< unoidl::MapCursor > const & theCursor):
            prefix(thePrefix), cursor(theCursor)
        { assert(theCursor.is()); }

        Position(
            rtl::OUString const & thePrefix,
            rtl::Reference< unoidl::ConstantGroupEntity > const &
                theConstantGroup):
            prefix(thePrefix), constantGroup(theConstantGroup),
            constantGroupIndex(constantGroup->getMembers().begin())
        { assert(theConstantGroup.is()); }

        Position(Position const & other):
            prefix(other.prefix), cursor(other.cursor),
            constantGroup(other.constantGroup)
        {
            if (constantGroup.is()) {
                constantGroupIndex = other.constantGroupIndex;
            }
        }

        rtl::OUString prefix;
        rtl::Reference< unoidl::MapCursor > cursor;
        rtl::Reference< unoidl::ConstantGroupEntity > constantGroup;
        std::vector< unoidl::ConstantGroupEntity::Member >::const_iterator
            constantGroupIndex;
    };

    rtl::Reference< cppuhelper::TypeManager > manager_;
    css::uno::Sequence< css::uno::TypeClass > types_;
    bool deep_;

    osl::Mutex mutex_;
    std::stack< Position > positions_;
    rtl::OUString current_;
};

css::uno::Reference< css::reflection::XTypeDescription >
Enumeration::nextTypeDescription()
    throw (css::container::NoSuchElementException, css::uno::RuntimeException)
{
    rtl::OUString name;
    {
        osl::MutexGuard g(mutex_);
        if (positions_.empty()) {
            throw css::container::NoSuchElementException(
                "exhausted XTypeDescriptionEnumeration",
                static_cast< cppu::OWeakObject * >(this));
        }
        name = current_;
        findNextMatch();
    }
    return manager_->resolve(name);
}

bool Enumeration::matches(css::uno::TypeClass tc) const {
    if (types_.getLength() == 0) {
        return true;
    }
    for (sal_Int32 i = 0; i != types_.getLength(); ++i) {
        if (types_[i] == tc) {
            return true;
        }
    }
    return false;
}

void Enumeration::findNextMatch() {
    try {
        for (;;) {
            assert(!positions_.empty());
            rtl::OUString name;
            if (positions_.top().cursor.is()) { // root or module
                rtl::Reference< unoidl::Entity > ent(
                    positions_.top().cursor->getNext(&name));
                if (!ent.is()) {
                    positions_.pop();
                    if (positions_.empty()) {
                        break;
                    }
                    continue;
                }
                name = positions_.top().prefix + name;
                css::uno::TypeClass tc;
                switch (ent->getSort()) {
                case unoidl::Entity::SORT_MODULE:
                    tc = css::uno::TypeClass_MODULE;
                    if (deep_) {
                        positions_.push(
                            Position(
                                makePrefix(name),
                                static_cast< unoidl::ModuleEntity * >(
                                    ent.get())->createCursor()));
                    }
                    break;
                case unoidl::Entity::SORT_ENUM_TYPE:
                    tc = css::uno::TypeClass_ENUM;
                    break;
                case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
                case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
                    tc = css::uno::TypeClass_STRUCT;
                    break;
                case unoidl::Entity::SORT_EXCEPTION_TYPE:
                    tc = css::uno::TypeClass_EXCEPTION;
                    break;
                case unoidl::Entity::SORT_INTERFACE_TYPE:
                    tc = css::uno::TypeClass_INTERFACE;
                    break;
                case unoidl::Entity::SORT_TYPEDEF:
                    tc = css::uno::TypeClass_TYPEDEF;
                    break;
                case unoidl::Entity::SORT_CONSTANT_GROUP:
                    tc = css::uno::TypeClass_CONSTANTS;
                    if (deep_ && matches(css::uno::TypeClass_CONSTANT)) {
                        positions_.push(
                            Position(
                                makePrefix(name),
                                static_cast< unoidl::ConstantGroupEntity * >(
                                    ent.get())));
                    }
                    break;
                case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
                case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
                    tc = css::uno::TypeClass_SERVICE;
                    break;
                case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
                case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
                    tc = css::uno::TypeClass_SINGLETON;
                    break;
                default:
                    for (;;) { std::abort(); } // this cannot happen
                }
                if (matches(tc)) {
                    current_ = name;
                    break;
                }
            } else { // constant group
                if (positions_.top().constantGroupIndex
                    == positions_.top().constantGroup->getMembers().end())
                {
                    positions_.pop();
                    if (positions_.empty()) {
                        break;
                    }
                    continue;
                }
                current_ = positions_.top().prefix
                    + positions_.top().constantGroupIndex++->name;
                break;
            }
        }
    } catch (unoidl::FileFormatException & e) {
        throw css::uno::DeploymentException(
            e.getUri() + ": " + e.getDetail(),
            static_cast< cppu::OWeakObject * >(this));
    }
}

}

cppuhelper::TypeManager::TypeManager():
    TypeManager_Base(*static_cast< osl::Mutex * >(this)),
    manager_(new unoidl::Manager)
{}

void cppuhelper::TypeManager::init(rtl::OUString const & rdbUris) {
    readRdbs(rdbUris);
}

css::uno::Any cppuhelper::TypeManager::find(rtl::OUString const & name) {
    //TODO: caching? (here or in unoidl::Manager?)
    struct Simple {
        char const * name; sal_Int32 length;
        css::uno::TypeClass typeClass;
    };
    static Simple const simple[] = {
        { RTL_CONSTASCII_STRINGPARAM("void"), css::uno::TypeClass_VOID },
        { RTL_CONSTASCII_STRINGPARAM("boolean"), css::uno::TypeClass_BOOLEAN },
        { RTL_CONSTASCII_STRINGPARAM("byte"), css::uno::TypeClass_BYTE },
        { RTL_CONSTASCII_STRINGPARAM("short"), css::uno::TypeClass_SHORT },
        { RTL_CONSTASCII_STRINGPARAM("unsigned short"),
          css::uno::TypeClass_UNSIGNED_SHORT },
        { RTL_CONSTASCII_STRINGPARAM("long"), css::uno::TypeClass_LONG },
        { RTL_CONSTASCII_STRINGPARAM("unsigned long"),
          css::uno::TypeClass_UNSIGNED_LONG },
        { RTL_CONSTASCII_STRINGPARAM("hyper"), css::uno::TypeClass_HYPER },
        { RTL_CONSTASCII_STRINGPARAM("unsigned hyper"),
          css::uno::TypeClass_UNSIGNED_HYPER },
        { RTL_CONSTASCII_STRINGPARAM("float"), css::uno::TypeClass_FLOAT },
        { RTL_CONSTASCII_STRINGPARAM("double"), css::uno::TypeClass_DOUBLE },
        { RTL_CONSTASCII_STRINGPARAM("char"), css::uno::TypeClass_CHAR },
        { RTL_CONSTASCII_STRINGPARAM("string"), css::uno::TypeClass_STRING },
        { RTL_CONSTASCII_STRINGPARAM("type"), css::uno::TypeClass_TYPE },
        { RTL_CONSTASCII_STRINGPARAM("any"), css::uno::TypeClass_ANY } };
    for (std::size_t i = 0; i != SAL_N_ELEMENTS(simple); ++i) {
        if (name.equalsAsciiL(simple[i].name, simple[i].length)) {
            return css::uno::makeAny<
                css::uno::Reference< css::reflection::XTypeDescription > >(
                    new SimpleTypeDescription(simple[i].typeClass, name));
        }
    }
    if (name.match("[]")) {
        return getSequenceType(name);
    }
    sal_Int32 i = name.indexOf('<');
    if (i != -1) {
        return getInstantiatedStruct(name, i);
    }
    i = name.indexOf("::");
    if (i != -1) {
        return getInterfaceMember(name, i);
    }
    rtl::Reference< unoidl::Entity > ent(findEntity(name));
    if (ent.is()) {
        return getNamed(name, ent);
    }
    i = name.lastIndexOf('.');
    if (i != -1) {
        rtl::OUString parent(name.copy(0, i));
        ent = findEntity(parent);
        if (ent.is()) {
            switch (ent->getSort()) {
            case unoidl::Entity::SORT_ENUM_TYPE:
                return getEnumMember(
                    static_cast< unoidl::EnumTypeEntity * >(ent.get()),
                    name.copy(i + 1));
            case unoidl::Entity::SORT_CONSTANT_GROUP:
                return getConstant(
                    parent,
                    static_cast< unoidl::ConstantGroupEntity * >(ent.get()),
                    name.copy(i + 1));
            default:
                break;
            }
        }
    }
    return css::uno::Any();
}

css::uno::Reference< css::reflection::XTypeDescription >
cppuhelper::TypeManager::resolve(rtl::OUString const & name) {
    css::uno::Reference< css::reflection::XTypeDescription > desc(
        find(name), css::uno::UNO_QUERY);
    if (!desc.is()) {
        throw css::uno::DeploymentException(
            "cannot resolve type \"" + name + "\"",
            static_cast< cppu::OWeakObject * >(this));
    }
    return desc;
}

cppuhelper::TypeManager::~TypeManager() throw () {}

void cppuhelper::TypeManager::disposing() {} //TODO

rtl::OUString cppuhelper::TypeManager::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return rtl::OUString(
        "com.sun.star.comp.cppuhelper.bootstrap.TypeManager");
}

sal_Bool cppuhelper::TypeManager::supportsService(
    rtl::OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< rtl::OUString >
cppuhelper::TypeManager::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > names(1);
    names[0] = "com.sun.star.reflection.TypeDescriptionManager"; //TODO
    return names;
}

css::uno::Any cppuhelper::TypeManager::getByHierarchicalName(
    rtl::OUString const & aName)
    throw (css::container::NoSuchElementException, css::uno::RuntimeException)
{
    css::uno::Any desc(find(aName));
    if (!desc.hasValue()) {
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return desc;
}

sal_Bool cppuhelper::TypeManager::hasByHierarchicalName(
    rtl::OUString const & aName)
    throw (css::uno::RuntimeException)
{
    return find(aName).hasValue();
}

css::uno::Type cppuhelper::TypeManager::getElementType()
    throw (css::uno::RuntimeException)
{
    return cppu::UnoType< rtl::OUString >::get();
}

sal_Bool cppuhelper::TypeManager::hasElements()
    throw (css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "TypeManager hasElements: method not supported",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Reference< css::container::XEnumeration >
cppuhelper::TypeManager::createEnumeration()
    throw (css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "TypeManager createEnumeration: method not supported",
        static_cast< cppu::OWeakObject * >(this));
}

sal_Bool cppuhelper::TypeManager::has(css::uno::Any const &)
    throw (css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "TypeManager has: method not supported",
        static_cast< cppu::OWeakObject * >(this));
}

void cppuhelper::TypeManager::insert(css::uno::Any const & aElement)
    throw (
        css::lang::IllegalArgumentException,
        css::container::ElementExistException, css::uno::RuntimeException)
{
    rtl::OUString uri;
    if (!(aElement >>= uri)) {
        throw css::lang::IllegalArgumentException(
            ("css.uno.theTypeDescriptionManager.insert expects a string URI"
             " argument"),
            static_cast< cppu::OWeakObject * >(this), 0);
    }
    //TODO: check for ElementExistException
    //TODO: check for consistency with existing data
    readRdbFile(uri, false);
}

void cppuhelper::TypeManager::remove(css::uno::Any const & aElement)
    throw (
        css::lang::IllegalArgumentException,
        css::container::NoSuchElementException, css::uno::RuntimeException)
{
    rtl::OUString uri;
    if (!(aElement >>= uri)) {
        throw css::lang::IllegalArgumentException(
            ("css.uno.theTypeDescriptionManager.remove expects a string URI"
             " argument"),
            static_cast< cppu::OWeakObject * >(this), 0);
    }
    //TODO: remove requests are silently ignored for now
}

css::uno::Reference< css::reflection::XTypeDescriptionEnumeration >
cppuhelper::TypeManager::createTypeDescriptionEnumeration(
    rtl::OUString const & moduleName,
    css::uno::Sequence< css::uno::TypeClass > const & types,
    css::reflection::TypeDescriptionSearchDepth depth)
    throw (
        css::reflection::NoSuchTypeNameException,
        css::reflection::InvalidTypeNameException,
        css::uno::RuntimeException)
{
    rtl::Reference< unoidl::MapCursor > cursor;
    try {
        cursor = manager_->createCursor(moduleName);
    } catch (unoidl::FileFormatException & e) {
        throw css::uno::DeploymentException(
            ("unoidl::FileFormatException for <" + e.getUri() + ">: "
             + e.getDetail()),
            static_cast< cppu::OWeakObject * >(this));
    }
    if (!cursor.is()) {
        //TODO: css::reflection::InvalidTypeNameException if moduleName names a
        // non-module
        throw css::reflection::NoSuchTypeNameException(
            moduleName, static_cast< cppu::OWeakObject * >(this));
    }
    return new Enumeration(
        this, makePrefix(moduleName), cursor, types,
        depth == css::reflection::TypeDescriptionSearchDepth_INFINITE);
}

void cppuhelper::TypeManager::readRdbs(rtl::OUString const & uris) {
    for (sal_Int32 i = 0; i != -1;) {
        rtl::OUString uri(uris.getToken(0, ' ', i));
        if (uri.isEmpty()) {
            continue;
        }
        bool optional;
        bool directory;
        cppu::decodeRdbUri(&uri, &optional, &directory);
        if (directory) {
            readRdbDirectory(uri, optional);
        } else {
            readRdbFile(uri, optional);
        }
    }
}

void cppuhelper::TypeManager::readRdbDirectory(
    rtl::OUString const & uri, bool optional)
{
    osl::Directory dir(uri);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << uri);
            return;
        }
        // fall through
    default:
        throw css::uno::DeploymentException(
            "Cannot open directory " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    for (;;) {
        rtl::OUString url;
        if (!cppu::nextDirectoryItem(dir, &url)) {
            break;
        }
        readRdbFile(url, false);
    }
}

void cppuhelper::TypeManager::readRdbFile(
    rtl::OUString const & uri, bool optional)
{
    rtl::Reference< unoidl::Provider > prov;
    try {
        prov = unoidl::loadProvider(manager_, uri);
    } catch (unoidl::NoSuchFileException &) {
        if (!optional) {
            throw css::uno::DeploymentException(
                uri + ": no such file",
                static_cast< cppu::OWeakObject * >(this));
        }
        SAL_INFO("cppuhelper", "Ignored optional " << uri);
    } catch (unoidl::FileFormatException & e) {
            throw css::uno::DeploymentException(
                ("unoidl::FileFormatException for <" + e.getUri() + ">: "
                 + e.getDetail()),
                static_cast< cppu::OWeakObject * >(this));
    }
    manager_->addProvider(prov);
}

css::uno::Any cppuhelper::TypeManager::getSequenceType(
    rtl::OUString const & name)
{
    assert(name.match("[]"));
    return css::uno::makeAny<
        css::uno::Reference< css::reflection::XTypeDescription > >(
            new SequenceTypeDescription(
                this, name, name.copy(std::strlen("[]"))));
}

css::uno::Any cppuhelper::TypeManager::getInstantiatedStruct(
    rtl::OUString const & name, sal_Int32 separator)
{
    assert(name.indexOf('<') == separator && separator != -1);
    rtl::Reference< unoidl::Entity > ent(findEntity(name.copy(0, separator)));
    if (!ent.is()
        || (ent->getSort()
            != unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE))
    {
        return css::uno::Any();
    }
    rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > ent2(
        static_cast< unoidl::PolymorphicStructTypeTemplateEntity * >(
            ent.get()));
    std::vector< rtl::OUString > args;
    sal_Int32 i = separator;
    do {
        ++i; // skip '<' or ','
        sal_Int32 j = i;
        for (sal_Int32 level = 0; j != name.getLength(); ++j) {
            sal_Unicode c = name[j];
            if (c == ',') {
                if (level == 0) {
                    break;
                }
            } else if (c == '<') {
                ++level;
            } else if (c == '>') {
                if (level == 0) {
                    break;
                }
                --level;
            }
        }
        if (j != name.getLength()) {
            args.push_back(name.copy(i, j - i));
        }
        i = j;
    } while (i != name.getLength() && name[i] != '>');
    if (i != name.getLength() - 1 || name[i] != '>'
        || args.size() != ent2->getTypeParameters().size())
    {
        return css::uno::Any();
    }
    return css::uno::makeAny<
        css::uno::Reference< css::reflection::XTypeDescription > >(
            new InstantiatedPolymorphicStructTypeDescription(
                this, name, ent2, args));
}

css::uno::Any cppuhelper::TypeManager::getInterfaceMember(
    rtl::OUString const & name, sal_Int32 separator)
{
    assert(name.indexOf("::") == separator && separator != -1);
    css::uno::Reference< css::reflection::XInterfaceTypeDescription2 > ifc(
        resolveTypedefs(find(name.copy(0, separator))), css::uno::UNO_QUERY);
    if (!ifc.is()) {
        return css::uno::Any();
    }
    rtl::OUString member(name.copy(separator + std::strlen("::")));
    css::uno::Sequence<
        css::uno::Reference<
            css::reflection::XInterfaceMemberTypeDescription > > mems(
                ifc->getMembers());
    for (sal_Int32 i = 0; i != mems.getLength(); ++i) {
        if (mems[i]->getMemberName() == member) {
            return css::uno::makeAny<
                css::uno::Reference< css::reflection::XTypeDescription > >(
                    mems[i]);
        }
    }
    return css::uno::Any();
}

css::uno::Any cppuhelper::TypeManager::getNamed(
    rtl::OUString const & name, rtl::Reference< unoidl::Entity > const & entity)
{
    assert(entity.is());
    switch (entity->getSort()) {
    case unoidl::Entity::SORT_MODULE:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new ModuleDescription(
                    this, name,
                    static_cast< unoidl::ModuleEntity * >(entity.get())));
    case unoidl::Entity::SORT_ENUM_TYPE:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new EnumTypeDescription(
                    name,
                    static_cast< unoidl::EnumTypeEntity * >(entity.get())));
    case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new PlainStructTypeDescription(
                    this, name,
                    static_cast< unoidl::PlainStructTypeEntity * >(
                        entity.get())));
    case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new PolymorphicStructTypeTemplateDescription(
                    this, name,
                    static_cast<
                        unoidl::PolymorphicStructTypeTemplateEntity * >(
                            entity.get())));
    case unoidl::Entity::SORT_EXCEPTION_TYPE:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new ExceptionTypeDescription(
                    this, name,
                    static_cast< unoidl::ExceptionTypeEntity * >(
                        entity.get())));
    case unoidl::Entity::SORT_INTERFACE_TYPE:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new InterfaceTypeDescription(
                    this, name,
                    static_cast< unoidl::InterfaceTypeEntity * >(
                        entity.get())));
    case unoidl::Entity::SORT_TYPEDEF:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new TypedefDescription(
                    this, name,
                    static_cast< unoidl::TypedefEntity * >(entity.get())));
    case unoidl::Entity::SORT_CONSTANT_GROUP:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new ConstantGroupDescription(
                    name,
                    static_cast< unoidl::ConstantGroupEntity * >(
                        entity.get())));
    case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new SingleInterfaceBasedServiceDescription(
                    this, name,
                    static_cast< unoidl::SingleInterfaceBasedServiceEntity * >(
                        entity.get())));
    case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new AccumulationBasedServiceDescription(
                    this, name,
                    static_cast< unoidl::AccumulationBasedServiceEntity * >(
                        entity.get())));
    case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new InterfaceBasedSingletonDescription(
                    this, name,
                    static_cast< unoidl::InterfaceBasedSingletonEntity * >(
                        entity.get())));
    case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new ServiceBasedSingletonDescription(
                    this, name,
                    static_cast< unoidl::ServiceBasedSingletonEntity * >(
                        entity.get())));
    default:
        for (;;) { std::abort(); } // this cannot happen
    }
}

css::uno::Any cppuhelper::TypeManager::getEnumMember(
    rtl::Reference< unoidl::EnumTypeEntity > const & entity,
    rtl::OUString const & member)
{
    for (std::vector< unoidl::EnumTypeEntity::Member >::const_iterator i(
             entity->getMembers().begin());
         i != entity->getMembers().end(); ++i)
    {
        if (i->name == member) {
            return css::uno::makeAny(i->value);
        }
    }
    return css::uno::Any();
}

css::uno::Any cppuhelper::TypeManager::getConstant(
    rtl::OUString const & constantGroupName,
    rtl::Reference< unoidl::ConstantGroupEntity > const & entity,
    rtl::OUString const & member)
{
    for (std::vector< unoidl::ConstantGroupEntity::Member >::const_iterator i(
             entity->getMembers().begin());
         i != entity->getMembers().end(); ++i)
    {
        if (i->name == member) {
            return css::uno::makeAny<
                css::uno::Reference< css::reflection::XTypeDescription > >(
                    new ConstantDescription(constantGroupName, *i));
        }
    }
    return css::uno::Any();
}

rtl::Reference< unoidl::Entity > cppuhelper::TypeManager::findEntity(
    rtl::OUString const & name)
{
    try {
        return manager_->findEntity(name);
    } catch (unoidl::FileFormatException & e) {
        throw css::uno::DeploymentException(
            ("unoidl::FileFormatException for <" + e.getUri() + ">: "
             + e.getDetail()),
            static_cast< cppu::OWeakObject * >(this));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
