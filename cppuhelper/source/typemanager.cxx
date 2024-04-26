/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <set>
#include <stack>
#include <string_view>
#include <utility>
#include <vector>
#include <algorithm>

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/reflection/NoSuchTypeNameException.hpp>
#include <com/sun/star/reflection/TypeDescriptionSearchDepth.hpp>
#include <com/sun/star/reflection/XConstantTypeDescription.hpp>
#include <com/sun/star/reflection/XConstantsTypeDescription.hpp>
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>
#include <com/sun/star/reflection/XIndirectTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceAttributeTypeDescription2.hpp>
#include <com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceTypeDescription2.hpp>
#include <com/sun/star/reflection/XModuleTypeDescription.hpp>
#include <com/sun/star/reflection/XPublished.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <com/sun/star/reflection/XSingletonTypeDescription2.hpp>
#include <com/sun/star/reflection/XStructTypeDescription.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <cppu/unotype.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/macros.h>
#include <sal/types.h>
#include <o3tl/string_view.hxx>

#include <unoidl/unoidl.hxx>

#include "paths.hxx"
#include "typemanager.hxx"

namespace {

OUString makePrefix(OUString const & name) {
    return name.isEmpty() ? name : name + ".";
}

css::uno::Any resolveTypedefs(css::uno::Any const & type) {
    for (css::uno::Any t(type);;) {
        css::uno::Reference< css::reflection::XIndirectTypeDescription > ind(
            type, css::uno::UNO_QUERY);
        if (!ind.is() || ind->getTypeClass() != css::uno::TypeClass_TYPEDEF) {
            return t;
        }
        t <<= ind->getReferencedType();
    }
}

class SimpleTypeDescription:
    public cppu::WeakImplHelper< css::reflection::XTypeDescription >
{
public:
    SimpleTypeDescription(
        css::uno::TypeClass typeClass, OUString name):
        typeClass_(typeClass), name_(std::move(name))
    {}

private:
    virtual ~SimpleTypeDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return typeClass_; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    css::uno::TypeClass typeClass_;
    OUString name_;
};

class SequenceTypeDescription:
    public cppu::WeakImplHelper< css::reflection::XIndirectTypeDescription >
{
public:
    SequenceTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name, OUString componentType):
        manager_(manager), name_(std::move(name)), componentType_(std::move(componentType))
    { assert(manager.is()); }

private:
    virtual ~SequenceTypeDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_SEQUENCE; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getReferencedType() override
    { return manager_->resolve(componentType_); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    OUString componentType_;
};

class PublishableDescription:
    public cppu::WeakImplHelper< css::reflection::XPublished >
{
protected:
    explicit PublishableDescription(bool published): published_(published) {}

    virtual ~PublishableDescription() override {}

private:
    virtual sal_Bool SAL_CALL isPublished() override
    { return published_; }

    bool published_;
};

class ModuleDescription:
    public cppu::WeakImplHelper< css::reflection::XModuleTypeDescription >
{
public:
    ModuleDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::ModuleEntity > const & entity):
        manager_(manager), name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~ModuleDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_MODULE; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMembers() override;

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::ModuleEntity > entity_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
ModuleDescription::getMembers() {
    try {
        std::vector< OUString > names(entity_->getMemberNames());
        assert(names.size() <= SAL_MAX_INT32);
        sal_Int32 n = static_cast< sal_Int32 >(names.size());
        css::uno::Sequence<
            css::uno::Reference< css::reflection::XTypeDescription > > s(n);
        auto r = asNonConstRange(s);
        for (sal_Int32 i = 0; i != n; ++i) {
            r[i] = manager_->resolve(makePrefix(name_) + names[i]);
        }
        return s;
    } catch (unoidl::FileFormatException & e) {
        throw css::uno::DeploymentException(
            e.getUri() + ": " + e.getDetail(),
            static_cast< cppu::OWeakObject * >(this));
    }
}

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XEnumTypeDescription >
EnumTypeDescription_Base;

class EnumTypeDescription: public EnumTypeDescription_Base {
public:
    EnumTypeDescription(
        OUString name,
        rtl::Reference< unoidl::EnumTypeEntity > const & entity):
        EnumTypeDescription_Base(entity->isPublished()), name_(std::move(name)),
        entity_(entity)
    { assert(entity.is()); }

private:
    virtual ~EnumTypeDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_ENUM; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual sal_Int32 SAL_CALL getDefaultEnumValue() override
    { return entity_->getMembers()[0].value; }

    virtual css::uno::Sequence< OUString > SAL_CALL getEnumNames() override;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getEnumValues() override;

    OUString name_;
    rtl::Reference< unoidl::EnumTypeEntity > entity_;
};

css::uno::Sequence< OUString > EnumTypeDescription::getEnumNames()
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence< OUString > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = entity_->getMembers()[i].name;
    }
    return s;
}

css::uno::Sequence< sal_Int32 > EnumTypeDescription::getEnumValues()
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence< sal_Int32 > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = entity_->getMembers()[i].value;
    }
    return s;
}

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XStructTypeDescription >
PlainStructTypeDescription_Base;

class PlainStructTypeDescription: public PlainStructTypeDescription_Base {
public:
    PlainStructTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::PlainStructTypeEntity > const & entity):
        PlainStructTypeDescription_Base(entity->isPublished()),
        manager_(manager), name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~PlainStructTypeDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_STRUCT; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() override {
        return entity_->getDirectBase().isEmpty()
            ? css::uno::Reference< css::reflection::XTypeDescription >()
            : manager_->resolve(entity_->getDirectBase());
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() override;

    virtual css::uno::Sequence< OUString > SAL_CALL getMemberNames() override;

    virtual css::uno::Sequence< OUString > SAL_CALL getTypeParameters() override
    { return css::uno::Sequence< OUString >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getTypeArguments() override {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XTypeDescription > >();
    }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::PlainStructTypeEntity > entity_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
PlainStructTypeDescription::getMemberTypes()
{
    assert(entity_->getDirectMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getDirectMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = manager_->resolve(entity_->getDirectMembers()[i].type);
    }
    return s;
}

css::uno::Sequence< OUString > PlainStructTypeDescription::getMemberNames()
{
    assert(entity_->getDirectMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getDirectMembers().size());
    css::uno::Sequence< OUString > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = entity_->getDirectMembers()[i].name;
    }
    return s;
}

class ParameterizedMemberTypeDescription:
    public cppu::WeakImplHelper< css::reflection::XTypeDescription >
{
public:
    explicit ParameterizedMemberTypeDescription(
        OUString typeParameterName):
        typeParameterName_(std::move(typeParameterName))
    {}

private:
    virtual ~ParameterizedMemberTypeDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_UNKNOWN; }

    virtual OUString SAL_CALL getName() override
    { return typeParameterName_; }

    OUString typeParameterName_;
};

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XStructTypeDescription >
PolymorphicStructTypeTemplateDescription_Base;

class PolymorphicStructTypeTemplateDescription:
    public PolymorphicStructTypeTemplateDescription_Base
{
public:
    PolymorphicStructTypeTemplateDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > const &
            entity):
        PolymorphicStructTypeTemplateDescription_Base(entity->isPublished()),
        manager_(manager), name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~PolymorphicStructTypeTemplateDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_STRUCT; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() override
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() override;

    virtual css::uno::Sequence< OUString > SAL_CALL getMemberNames() override;

    virtual css::uno::Sequence< OUString > SAL_CALL getTypeParameters() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getTypeArguments() override {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XTypeDescription > >();
    }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > entity_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
PolymorphicStructTypeTemplateDescription::getMemberTypes()
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = entity_->getMembers()[i].parameterized
            ? new ParameterizedMemberTypeDescription(
                entity_->getMembers()[i].type)
            : manager_->resolve(entity_->getMembers()[i].type);
    }
    return s;
}

css::uno::Sequence< OUString >
PolymorphicStructTypeTemplateDescription::getMemberNames()
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence< OUString > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = entity_->getMembers()[i].name;
    }
    return s;
}

css::uno::Sequence< OUString >
PolymorphicStructTypeTemplateDescription::getTypeParameters()
{
    assert(entity_->getTypeParameters().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getTypeParameters().size());
    css::uno::Sequence< OUString > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = entity_->getTypeParameters()[i];
    }
    return s;
}

class InstantiatedPolymorphicStructTypeDescription:
    public cppu::WeakImplHelper< css::reflection::XStructTypeDescription >
{
public:
    InstantiatedPolymorphicStructTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > const &
            entity,
        std::vector< OUString >&& arguments):
        manager_(manager), name_(std::move(name)), entity_(entity), arguments_(std::move(arguments))
    {
        assert(manager.is());
        assert(entity.is());
        assert(arguments_.size() == entity->getTypeParameters().size());
    }

private:
    virtual ~InstantiatedPolymorphicStructTypeDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_STRUCT; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() override
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() override;

    virtual css::uno::Sequence< OUString > SAL_CALL getMemberNames() override;

    virtual css::uno::Sequence< OUString > SAL_CALL getTypeParameters() override
    { return css::uno::Sequence< OUString >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getTypeArguments() override;

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > entity_;
    std::vector< OUString > arguments_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InstantiatedPolymorphicStructTypeDescription::getMemberTypes()
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        OUString type(entity_->getMembers()[i].type);
        if (entity_->getMembers()[i].parameterized) {
            auto j = std::find(entity_->getTypeParameters().begin(), entity_->getTypeParameters().end(), type);
            if (j != entity_->getTypeParameters().end()) {
                type = arguments_[j - entity_->getTypeParameters().begin()];
                goto found;
            }
            assert(false); // this cannot happen                         //TODO!
        found:;
        }
        r[i] = manager_->resolve(type);
    }
    return s;
}

css::uno::Sequence< OUString >
InstantiatedPolymorphicStructTypeDescription::getMemberNames()
{
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence< OUString > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = entity_->getMembers()[i].name;
    }
    return s;
}
css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InstantiatedPolymorphicStructTypeDescription::getTypeArguments()
{
    assert(arguments_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(arguments_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = manager_->resolve(arguments_[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XCompoundTypeDescription >
ExceptionTypeDescription_Base;

class ExceptionTypeDescription: public ExceptionTypeDescription_Base {
public:
    ExceptionTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::ExceptionTypeEntity > const & entity):
        ExceptionTypeDescription_Base(entity->isPublished()), manager_(manager),
        name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~ExceptionTypeDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_EXCEPTION; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() override {
        return entity_->getDirectBase().isEmpty()
            ? css::uno::Reference< css::reflection::XTypeDescription >()
            : manager_->resolve(entity_->getDirectBase());
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() override;

    virtual css::uno::Sequence< OUString > SAL_CALL getMemberNames() override;

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::ExceptionTypeEntity > entity_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
ExceptionTypeDescription::getMemberTypes() {
    assert(entity_->getDirectMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getDirectMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = manager_->resolve(entity_->getDirectMembers()[i].type);
    }
    return s;
}

css::uno::Sequence< OUString > ExceptionTypeDescription::getMemberNames()
{
    assert(entity_->getDirectMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getDirectMembers().size());
    css::uno::Sequence< OUString > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = entity_->getDirectMembers()[i].name;
    }
    return s;
}

class AttributeDescription:
    public cppu::WeakImplHelper<
        css::reflection::XInterfaceAttributeTypeDescription2 >
{
public:
    AttributeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        unoidl::InterfaceTypeEntity::Attribute  attribute,
        sal_Int32 position):
        manager_(manager), name_(std::move(name)), attribute_(std::move(attribute)),
        position_(position)
    { assert(manager.is()); }

private:
    virtual ~AttributeDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_INTERFACE_ATTRIBUTE; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual OUString SAL_CALL getMemberName() override
    { return attribute_.name; }

    virtual sal_Int32 SAL_CALL getPosition() override
    { return position_; }

    virtual sal_Bool SAL_CALL isReadOnly() override
    { return attribute_.readOnly; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() override
    { return manager_->resolve(attribute_.type); }

    virtual sal_Bool SAL_CALL isBound() override
    { return attribute_.bound; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
    SAL_CALL getGetExceptions() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
    SAL_CALL getSetExceptions() override;

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    unoidl::InterfaceTypeEntity::Attribute attribute_;
    sal_Int32 position_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
AttributeDescription::getGetExceptions() {
    assert(attribute_.getExceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(attribute_.getExceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i].set(
            manager_->resolve(attribute_.getExceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
AttributeDescription::getSetExceptions() {
    assert(attribute_.setExceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(attribute_.setExceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i].set(
            manager_->resolve(attribute_.setExceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

class MethodParameter:
    public cppu::WeakImplHelper< css::reflection::XMethodParameter >
{
public:
    MethodParameter(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        unoidl::InterfaceTypeEntity::Method::Parameter parameter,
        sal_Int32 position):
        manager_(manager), parameter_(std::move(parameter)), position_(position)
    { assert(manager.is()); }

private:
    virtual ~MethodParameter() override {}

    virtual OUString SAL_CALL getName() override
    { return parameter_.name; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() override
    { return manager_->resolve(parameter_.type); }

    virtual sal_Bool SAL_CALL isIn() override {
        return
            (parameter_.direction
             == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
            || (parameter_.direction
                == unoidl::InterfaceTypeEntity::Method::Parameter::
                    DIRECTION_IN_OUT);
    }

    virtual sal_Bool SAL_CALL isOut() override {
        return
            (parameter_.direction
             == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_OUT)
            || (parameter_.direction
                == unoidl::InterfaceTypeEntity::Method::Parameter::
                    DIRECTION_IN_OUT);
    }

    virtual sal_Int32 SAL_CALL getPosition() override
    { return position_; }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    unoidl::InterfaceTypeEntity::Method::Parameter parameter_;
    sal_Int32 position_;
};

class MethodDescription:
    public cppu::WeakImplHelper<
        css::reflection::XInterfaceMethodTypeDescription >
{
public:
    MethodDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        unoidl::InterfaceTypeEntity::Method method, sal_Int32 position):
        manager_(manager), name_(std::move(name)), method_(std::move(method)), position_(position)
    { assert(manager.is()); }

private:
    virtual ~MethodDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_INTERFACE_METHOD; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual OUString SAL_CALL getMemberName() override
    { return method_.name; }

    virtual sal_Int32 SAL_CALL getPosition() override
    { return position_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getReturnType() override
    { return manager_->resolve(method_.returnType); }

    virtual sal_Bool SAL_CALL isOneway() override
    { return false; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XMethodParameter > >
    SAL_CALL getParameters() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getExceptions() override;

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    unoidl::InterfaceTypeEntity::Method method_;
    sal_Int32 position_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XMethodParameter > >
MethodDescription::getParameters() {
    assert(method_.parameters.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(method_.parameters.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XMethodParameter > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = new MethodParameter(manager_, method_.parameters[i], i);
    }
    return s;
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
MethodDescription::getExceptions() {
    assert(method_.exceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(method_.exceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = manager_->resolve(method_.exceptions[i]);
    }
    return s;
}

class BaseOffset {
public:
    explicit BaseOffset(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
            const & description);

    BaseOffset(const BaseOffset&) = delete;
    const BaseOffset& operator=(const BaseOffset&) = delete;

    sal_Int32 get() const { return offset_; }

private:
    void calculateBases(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
            const & description);

    void calculate(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
            const & description);

    std::set< OUString > set_;
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
    const css::uno::Sequence<
        css::uno::Reference < css::reflection::XTypeDescription > > bases(
            description->getBaseTypes());
    for (const auto & i : bases) {
        calculate(
            css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >(
                resolveTypedefs(css::uno::Any(i)),
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

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XInterfaceTypeDescription2 >
InterfaceTypeDescription_Base;

class InterfaceTypeDescription: public InterfaceTypeDescription_Base {
public:
    InterfaceTypeDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::InterfaceTypeEntity > const & entity):
        InterfaceTypeDescription_Base(entity->isPublished()), manager_(manager),
        name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~InterfaceTypeDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_INTERFACE; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() override {
        return entity_->getDirectMandatoryBases().empty()
            ? css::uno::Reference< css::reflection::XTypeDescription >()
            : manager_->resolve(entity_->getDirectMandatoryBases()[0].name);
    }

    virtual css::uno::Uik SAL_CALL getUik() override
    { return css::uno::Uik(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference<
             css::reflection::XInterfaceMemberTypeDescription > >
    SAL_CALL getMembers() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getBaseTypes() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getOptionalBaseTypes() override;

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::InterfaceTypeEntity > entity_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XInterfaceMemberTypeDescription > >
InterfaceTypeDescription::getMembers() {
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
    auto r = asNonConstRange(s);
    sal_Int32 off = BaseOffset(this).get();
    for (sal_Int32 i = 0; i != n1; ++i) {
        r[i] = new AttributeDescription(
            manager_, name_ + "::" + entity_->getDirectAttributes()[i].name,
            entity_->getDirectAttributes()[i], off + i);
    }
    for (sal_Int32 i = 0; i != n2; ++i) {
        r[n1 + i] = new MethodDescription(
            manager_, name_ + "::" + entity_->getDirectMethods()[i].name,
            entity_->getDirectMethods()[i], off + n1 + i);
    }
    return s;
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InterfaceTypeDescription::getBaseTypes() {
    assert(entity_->getDirectMandatoryBases().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectMandatoryBases().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = manager_->resolve(entity_->getDirectMandatoryBases()[i].name);
    }
    return s;
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InterfaceTypeDescription::getOptionalBaseTypes()
{
    assert(entity_->getDirectOptionalBases().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectOptionalBases().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = manager_->resolve(entity_->getDirectOptionalBases()[i].name);
    }
    return s;
}

class ConstantDescription:
    public cppu::WeakImplHelper< css::reflection::XConstantTypeDescription >
{
public:
    ConstantDescription(
        OUString const & constantGroupName,
        unoidl::ConstantGroupEntity::Member const & member);

private:
    virtual ~ConstantDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_CONSTANT; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Any SAL_CALL getConstantValue() override
    { return value_; }

    OUString name_;
    css::uno::Any value_;
};

ConstantDescription::ConstantDescription(
    OUString const & constantGroupName,
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

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XConstantsTypeDescription >
ConstantGroupDescription_Base;

class ConstantGroupDescription: public ConstantGroupDescription_Base {
public:
    ConstantGroupDescription(
        OUString name,
        rtl::Reference< unoidl::ConstantGroupEntity > const & entity):
        ConstantGroupDescription_Base(entity->isPublished()), name_(std::move(name)),
        entity_(entity)
    { assert(entity.is()); }

private:
    virtual ~ConstantGroupDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_CONSTANTS; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XConstantTypeDescription > >
    SAL_CALL getConstants() override;

    OUString name_;
    rtl::Reference< unoidl::ConstantGroupEntity > entity_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XConstantTypeDescription > >
ConstantGroupDescription::getConstants() {
    assert(entity_->getMembers().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getMembers().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XConstantTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = new ConstantDescription(name_, entity_->getMembers()[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XIndirectTypeDescription >
TypedefDescription_Base;

class TypedefDescription: public TypedefDescription_Base {
public:
    TypedefDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::TypedefEntity > const & entity):
        TypedefDescription_Base(entity->isPublished()), manager_(manager),
        name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~TypedefDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_TYPEDEF; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getReferencedType() override
    { return manager_->resolve(entity_->getType()); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::TypedefEntity > entity_;
};

class ConstructorParameter:
    public cppu::WeakImplHelper< css::reflection::XParameter >
{
public:
    ConstructorParameter(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter parameter,
        sal_Int32 position):
        manager_(manager), parameter_(std::move(parameter)), position_(position)
    { assert(manager.is()); }

private:
    virtual ~ConstructorParameter() override {}

    virtual OUString SAL_CALL getName() override
    { return parameter_.name; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() override
    { return manager_->resolve(parameter_.type); }

    virtual sal_Bool SAL_CALL isIn() override
    { return true; }

    virtual sal_Bool SAL_CALL isOut() override
    { return false; }

    virtual sal_Int32 SAL_CALL getPosition() override
    { return position_; }

    virtual sal_Bool SAL_CALL isRestParameter() override
    { return parameter_.rest; }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter
        parameter_;
    sal_Int32 position_;
};

class ConstructorDescription:
    public cppu::WeakImplHelper<
        css::reflection::XServiceConstructorDescription >
{
public:
    ConstructorDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        unoidl::SingleInterfaceBasedServiceEntity::Constructor constructor):
        manager_(manager), constructor_(std::move(constructor))
    { assert(manager.is()); }

private:
    virtual ~ConstructorDescription() override {}

    virtual sal_Bool SAL_CALL isDefaultConstructor() override
    { return constructor_.defaultConstructor; }

    virtual OUString SAL_CALL getName() override
    { return constructor_.name; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XParameter > >
    SAL_CALL getParameters() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
    SAL_CALL getExceptions() override;

    rtl::Reference< cppuhelper::TypeManager > manager_;
    unoidl::SingleInterfaceBasedServiceEntity::Constructor constructor_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XParameter > >
ConstructorDescription::getParameters() {
    assert(constructor_.parameters.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(constructor_.parameters.size());
    css::uno::Sequence< css::uno::Reference< css::reflection::XParameter > > s(
        n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = new ConstructorParameter(
            manager_, constructor_.parameters[i], i);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
ConstructorDescription::getExceptions() {
    assert(constructor_.exceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(constructor_.exceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i].set(
            manager_->resolve(constructor_.exceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XServiceTypeDescription2 >
SingleInterfaceBasedServiceDescription_Base;

class SingleInterfaceBasedServiceDescription:
    public SingleInterfaceBasedServiceDescription_Base
{
public:
    SingleInterfaceBasedServiceDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > const &
            entity):
        SingleInterfaceBasedServiceDescription_Base(entity->isPublished()),
        manager_(manager), name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~SingleInterfaceBasedServiceDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_SERVICE; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getMandatoryServices() override
    {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XServiceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getOptionalServices() override
    {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XServiceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getMandatoryInterfaces() override
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XInterfaceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getOptionalInterfaces() override
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XInterfaceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XPropertyTypeDescription > >
    SAL_CALL getProperties() override
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XPropertyTypeDescription > >();
    }

    virtual sal_Bool SAL_CALL isSingleInterfaceBased() override
    { return true; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getInterface() override
    { return manager_->resolve(entity_->getBase()); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
    SAL_CALL getConstructors() override;

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > entity_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XServiceConstructorDescription > >
SingleInterfaceBasedServiceDescription::getConstructors()
{
    assert(entity_->getConstructors().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(entity_->getConstructors().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
            s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = new ConstructorDescription(
            manager_, entity_->getConstructors()[i]);
    }
    return s;
}

class PropertyDescription:
    public cppu::WeakImplHelper< css::reflection::XPropertyTypeDescription >
{
public:
    PropertyDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        unoidl::AccumulationBasedServiceEntity::Property property):
        manager_(manager), property_(std::move(property))
    { assert(manager.is()); }

private:
    virtual ~PropertyDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_PROPERTY; }

    virtual OUString SAL_CALL getName() override
    { return property_.name; }

    virtual sal_Int16 SAL_CALL getPropertyFlags() override
    { return property_.attributes; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getPropertyTypeDescription() override
    { return manager_->resolve(property_.type); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    unoidl::AccumulationBasedServiceEntity::Property property_;
};

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XServiceTypeDescription2 >
AccumulationBasedServiceDescription_Base;

class AccumulationBasedServiceDescription:
    public AccumulationBasedServiceDescription_Base
{
public:
    AccumulationBasedServiceDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::AccumulationBasedServiceEntity > const &
            entity):
        AccumulationBasedServiceDescription_Base(entity->isPublished()),
        manager_(manager), name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~AccumulationBasedServiceDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_SERVICE; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getMandatoryServices() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getOptionalServices() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getMandatoryInterfaces() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getOptionalInterfaces() override;

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XPropertyTypeDescription > >
    SAL_CALL getProperties() override;

    virtual sal_Bool SAL_CALL isSingleInterfaceBased() override
    { return false; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getInterface() override
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
    SAL_CALL getConstructors() override
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XServiceConstructorDescription > >();
    }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::AccumulationBasedServiceEntity > entity_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XServiceTypeDescription > >
AccumulationBasedServiceDescription::getMandatoryServices()
{
    assert(entity_->getDirectMandatoryBaseServices().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectMandatoryBaseServices().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i].set(
            manager_->resolve(
                entity_->getDirectMandatoryBaseServices()[i].name),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XServiceTypeDescription > >
AccumulationBasedServiceDescription::getOptionalServices()
{
    assert(entity_->getDirectOptionalBaseServices().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectOptionalBaseServices().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i].set(
            manager_->resolve(entity_->getDirectOptionalBaseServices()[i].name),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
AccumulationBasedServiceDescription::getMandatoryInterfaces()
{
    assert(entity_->getDirectMandatoryBaseInterfaces().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectMandatoryBaseInterfaces().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > > s(
            n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i].set(
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
{
    assert(entity_->getDirectOptionalBaseInterfaces().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectOptionalBaseInterfaces().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > > s(
            n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i].set(
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
{
    assert(entity_->getDirectProperties().size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        entity_->getDirectProperties().size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XPropertyTypeDescription > > s(n);
    auto r = asNonConstRange(s);
    for (sal_Int32 i = 0; i != n; ++i) {
        r[i] = new PropertyDescription(
            manager_, entity_->getDirectProperties()[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XSingletonTypeDescription2 >
InterfaceBasedSingletonDescription_Base;

class InterfaceBasedSingletonDescription:
    public InterfaceBasedSingletonDescription_Base
{
public:
    InterfaceBasedSingletonDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::InterfaceBasedSingletonEntity > const & entity):
        InterfaceBasedSingletonDescription_Base(entity->isPublished()),
        manager_(manager), name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~InterfaceBasedSingletonDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_SINGLETON; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Reference< css::reflection::XServiceTypeDescription >
    SAL_CALL getService() override
    {
        return
            css::uno::Reference< css::reflection::XServiceTypeDescription >();
    }

    virtual sal_Bool SAL_CALL isInterfaceBased() override
    { return true; }

    virtual css::uno::Reference< css::reflection::XTypeDescription >
    SAL_CALL getInterface() override
    { return manager_->resolve(entity_->getBase()); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::InterfaceBasedSingletonEntity > entity_;
};

typedef cppu::ImplInheritanceHelper<
    PublishableDescription, css::reflection::XSingletonTypeDescription2 >
ServiceBasedSingletonDescription_Base;

class ServiceBasedSingletonDescription:
    public ServiceBasedSingletonDescription_Base
{
public:
    ServiceBasedSingletonDescription(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString name,
        rtl::Reference< unoidl::ServiceBasedSingletonEntity > const & entity):
        ServiceBasedSingletonDescription_Base(entity->isPublished()),
        manager_(manager), name_(std::move(name)), entity_(entity)
    { assert(manager.is()); assert(entity.is()); }

private:
    virtual ~ServiceBasedSingletonDescription() override {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass() override
    { return css::uno::TypeClass_SINGLETON; }

    virtual OUString SAL_CALL getName() override
    { return name_; }

    virtual css::uno::Reference< css::reflection::XServiceTypeDescription >
    SAL_CALL getService() override
    {
        return css::uno::Reference< css::reflection::XServiceTypeDescription >(
            manager_->resolve(entity_->getBase()), css::uno::UNO_QUERY_THROW);
    }

    virtual sal_Bool SAL_CALL isInterfaceBased() override
    { return false; }

    virtual css::uno::Reference< css::reflection::XTypeDescription >
    SAL_CALL getInterface() override
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    rtl::Reference< cppuhelper::TypeManager > manager_;
    OUString name_;
    rtl::Reference< unoidl::ServiceBasedSingletonEntity > entity_;
};

class Enumeration:
    public cppu::WeakImplHelper< css::reflection::XTypeDescriptionEnumeration >
{
public:
    Enumeration(
        rtl::Reference< cppuhelper::TypeManager > const & manager,
        OUString const & prefix,
        rtl::Reference< unoidl::MapCursor > const & cursor,
        css::uno::Sequence< css::uno::TypeClass > const & types, bool deep):
        manager_(manager), types_(types), deep_(deep)
    {
        assert(manager.is());
        positions_.push(Position(prefix, cursor));
        findNextMatch();
    }

private:
    virtual ~Enumeration() override {}

    virtual sal_Bool SAL_CALL hasMoreElements() override
    { return !positions_.empty(); }

    virtual css::uno::Any SAL_CALL nextElement() override
    { return css::uno::Any(nextTypeDescription()); }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    nextTypeDescription() override;

    bool matches(css::uno::TypeClass tc) const;

    void findNextMatch();

    struct Position {
        Position(
            OUString thePrefix,
            rtl::Reference< unoidl::MapCursor > const & theCursor):
            prefix(std::move(thePrefix)), cursor(theCursor)
        { assert(theCursor.is()); }

        Position(
            OUString thePrefix,
            rtl::Reference< unoidl::ConstantGroupEntity > const &
                theConstantGroup):
            prefix(std::move(thePrefix)), constantGroup(theConstantGroup),
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

        OUString prefix;
        rtl::Reference< unoidl::MapCursor > cursor;
        rtl::Reference< unoidl::ConstantGroupEntity > constantGroup;
        std::vector< unoidl::ConstantGroupEntity::Member >::const_iterator
            constantGroupIndex;
    };

    rtl::Reference< cppuhelper::TypeManager > manager_;
    css::uno::Sequence< css::uno::TypeClass > types_;
    bool deep_;

    std::mutex mutex_;
    std::stack< Position, std::vector<Position> > positions_;
    OUString current_;
};

css::uno::Reference< css::reflection::XTypeDescription >
Enumeration::nextTypeDescription()
{
    OUString name;
    {
        std::scoped_lock g(mutex_);
        if (positions_.empty()) {
            throw css::container::NoSuchElementException(
                u"exhausted XTypeDescriptionEnumeration"_ustr,
                static_cast< cppu::OWeakObject * >(this));
        }
        name = current_;
        findNextMatch();
    }
    return manager_->resolve(name);
}

bool Enumeration::matches(css::uno::TypeClass tc) const {
    if (!types_.hasElements()) {
        return true;
    }

    return std::any_of(types_.begin(), types_.end(), [&tc](const auto& i) { return i == tc; });
}

void Enumeration::findNextMatch() {
    try {
        for (;;) {
            assert(!positions_.empty());
            OUString name;
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
    manager_(new unoidl::Manager)
{}

css::uno::Any cppuhelper::TypeManager::find(OUString const & name) {
    //TODO: caching? (here or in unoidl::Manager?)
    static constexpr std::pair<std::u16string_view, css::uno::TypeClass> const simple[] = {
        { u"void", css::uno::TypeClass_VOID },
        { u"boolean", css::uno::TypeClass_BOOLEAN },
        { u"byte", css::uno::TypeClass_BYTE },
        { u"short", css::uno::TypeClass_SHORT },
        { u"unsigned short", css::uno::TypeClass_UNSIGNED_SHORT },
        { u"long", css::uno::TypeClass_LONG },
        { u"unsigned long", css::uno::TypeClass_UNSIGNED_LONG },
        { u"hyper", css::uno::TypeClass_HYPER },
        { u"unsigned hyper", css::uno::TypeClass_UNSIGNED_HYPER },
        { u"float", css::uno::TypeClass_FLOAT },
        { u"double", css::uno::TypeClass_DOUBLE },
        { u"char", css::uno::TypeClass_CHAR },
        { u"string", css::uno::TypeClass_STRING },
        { u"type", css::uno::TypeClass_TYPE },
        { u"any", css::uno::TypeClass_ANY } };
    for (const auto& [ rName, rTypeClass ] : simple) {
        if (name == rName) {
            return css::uno::Any(
                css::uno::Reference< css::reflection::XTypeDescription >(
                    new SimpleTypeDescription(rTypeClass, name)));
        }
    }
    if (name.startsWith("[]")) {
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
        OUString parent(name.copy(0, i));
        ent = findEntity(parent);
        if (ent.is()) {
            switch (ent->getSort()) {
            case unoidl::Entity::SORT_ENUM_TYPE:
                return getEnumMember(
                    static_cast< unoidl::EnumTypeEntity * >(ent.get()),
                    name.subView(i + 1));
            case unoidl::Entity::SORT_CONSTANT_GROUP:
                return getConstant(
                    parent,
                    static_cast< unoidl::ConstantGroupEntity * >(ent.get()),
                    name.subView(i + 1));
            default:
                break;
            }
        }
    }
    return css::uno::Any();
}

css::uno::Reference< css::reflection::XTypeDescription >
cppuhelper::TypeManager::resolve(OUString const & name) {
    css::uno::Reference< css::reflection::XTypeDescription > desc(
        find(name), css::uno::UNO_QUERY);
    if (!desc.is()) {
        throw css::uno::DeploymentException(
            "cannot resolve type \"" + name + "\"",
            static_cast< cppu::OWeakObject * >(this));
    }
    return desc;
}

cppuhelper::TypeManager::~TypeManager() noexcept {}

OUString cppuhelper::TypeManager::getImplementationName()
{
    return
        u"com.sun.star.comp.cppuhelper.bootstrap.TypeManager"_ustr;
}

sal_Bool cppuhelper::TypeManager::supportsService(
    OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString >
cppuhelper::TypeManager::getSupportedServiceNames()
{
    return { u"com.sun.star.reflection.TypeDescriptionManager"_ustr }; //TODO
}

css::uno::Any cppuhelper::TypeManager::getByHierarchicalName(
    OUString const & aName)
{
    css::uno::Any desc(find(aName));
    if (!desc.hasValue()) {
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return desc;
}

sal_Bool cppuhelper::TypeManager::hasByHierarchicalName(
    OUString const & aName)
{
    return find(aName).hasValue();
}

css::uno::Type cppuhelper::TypeManager::getElementType()
{
    return cppu::UnoType< OUString >::get();
}

sal_Bool cppuhelper::TypeManager::hasElements()
{
    throw css::uno::RuntimeException(
        u"TypeManager hasElements: method not supported"_ustr,
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Reference< css::container::XEnumeration >
cppuhelper::TypeManager::createEnumeration()
{
    throw css::uno::RuntimeException(
        u"TypeManager createEnumeration: method not supported"_ustr,
        static_cast< cppu::OWeakObject * >(this));
}

sal_Bool cppuhelper::TypeManager::has(css::uno::Any const &)
{
    throw css::uno::RuntimeException(
        u"TypeManager has: method not supported"_ustr,
        static_cast< cppu::OWeakObject * >(this));
}

void cppuhelper::TypeManager::insert(css::uno::Any const & aElement)
{
    OUString uri;
    if (!(aElement >>= uri)) {
        throw css::lang::IllegalArgumentException(
            (u"css.uno.theTypeDescriptionManager.insert expects a string URI"
             " argument"_ustr),
            static_cast< cppu::OWeakObject * >(this), 0);
    }
    //TODO: check for ElementExistException
    //TODO: check for consistency with existing data
    readRdbFile(uri, false);
}

void cppuhelper::TypeManager::remove(css::uno::Any const & aElement)
{
    OUString uri;
    if (!(aElement >>= uri)) {
        throw css::lang::IllegalArgumentException(
            (u"css.uno.theTypeDescriptionManager.remove expects a string URI"
             " argument"_ustr),
            static_cast< cppu::OWeakObject * >(this), 0);
    }
    //TODO: remove requests are silently ignored for now
}

css::uno::Reference< css::reflection::XTypeDescriptionEnumeration >
cppuhelper::TypeManager::createTypeDescriptionEnumeration(
    OUString const & moduleName,
    css::uno::Sequence< css::uno::TypeClass > const & types,
    css::reflection::TypeDescriptionSearchDepth depth)
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

void cppuhelper::TypeManager::init(std::u16string_view rdbUris) {
    for (sal_Int32 i = 0; i != -1;) {
        std::u16string_view uri(o3tl::getToken(rdbUris, 0, ' ', i));
        if (uri.empty()) {
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
    std::u16string_view uri, bool optional)
{
    osl::Directory dir = OUString(uri);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << OUString(uri));
            return;
        }
        [[fallthrough]];
    default:
        throw css::uno::DeploymentException(
            OUString::Concat("Cannot open directory ") + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    for (;;) {
        OUString url;
        if (!cppu::nextDirectoryItem(dir, &url)) {
            break;
        }
        readRdbFile(url, false);
    }
}

void cppuhelper::TypeManager::readRdbFile(
    std::u16string_view uri, bool optional)
{
    try {
        manager_->addProvider(OUString(uri));
    } catch (unoidl::NoSuchFileException &) {
        if (!optional) {
            throw css::uno::DeploymentException(
                OUString::Concat(uri) + ": no such file",
                static_cast< cppu::OWeakObject * >(this));
        }
        SAL_INFO("cppuhelper", "Ignored optional " << OUString(uri));
    } catch (unoidl::FileFormatException & e) {
            throw css::uno::DeploymentException(
                ("unoidl::FileFormatException for <" + e.getUri() + ">: "
                 + e.getDetail()),
                static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Any cppuhelper::TypeManager::getSequenceType(
    OUString const & name)
{
    assert(name.startsWith("[]"));
    return css::uno::Any(
        css::uno::Reference< css::reflection::XTypeDescription >(
            new SequenceTypeDescription(
                this, name, name.copy(std::strlen("[]")))));
}

css::uno::Any cppuhelper::TypeManager::getInstantiatedStruct(
    OUString const & name, sal_Int32 separator)
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
    std::vector< OUString > args;
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
    return css::uno::Any(
        css::uno::Reference< css::reflection::XTypeDescription >(
            new InstantiatedPolymorphicStructTypeDescription(
                this, name, ent2, std::move(args))));
}

css::uno::Any cppuhelper::TypeManager::getInterfaceMember(
    std::u16string_view name, std::size_t separator)
{
    assert(name.find(u"::") == separator && separator != std::u16string_view::npos);
    css::uno::Reference< css::reflection::XInterfaceTypeDescription2 > ifc(
        resolveTypedefs(find(OUString(name.substr(0, separator)))), css::uno::UNO_QUERY);
    if (!ifc.is()) {
        return css::uno::Any();
    }
    std::u16string_view member = name.substr(separator + std::strlen("::"));
    const css::uno::Sequence<
        css::uno::Reference<
            css::reflection::XInterfaceMemberTypeDescription > > mems(
                ifc->getMembers());
    for (const auto & m : mems) {
        if (m->getMemberName() == member) {
            return css::uno::Any(
                css::uno::Reference< css::reflection::XTypeDescription >(m));
        }
    }
    return css::uno::Any();
}

css::uno::Any cppuhelper::TypeManager::getNamed(
    OUString const & name, rtl::Reference< unoidl::Entity > const & entity)
{
    assert(entity.is());
    switch (entity->getSort()) {
    case unoidl::Entity::SORT_MODULE:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new ModuleDescription(
                    this, name,
                    static_cast< unoidl::ModuleEntity * >(entity.get()))));
    case unoidl::Entity::SORT_ENUM_TYPE:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new EnumTypeDescription(
                    name,
                    static_cast< unoidl::EnumTypeEntity * >(entity.get()))));
    case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new PlainStructTypeDescription(
                    this, name,
                    static_cast< unoidl::PlainStructTypeEntity * >(
                        entity.get()))));
    case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new PolymorphicStructTypeTemplateDescription(
                    this, name,
                    static_cast<
                        unoidl::PolymorphicStructTypeTemplateEntity * >(
                            entity.get()))));
    case unoidl::Entity::SORT_EXCEPTION_TYPE:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new ExceptionTypeDescription(
                    this, name,
                    static_cast< unoidl::ExceptionTypeEntity * >(
                        entity.get()))));
    case unoidl::Entity::SORT_INTERFACE_TYPE:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new InterfaceTypeDescription(
                    this, name,
                    static_cast< unoidl::InterfaceTypeEntity * >(
                        entity.get()))));
    case unoidl::Entity::SORT_TYPEDEF:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new TypedefDescription(
                    this, name,
                    static_cast< unoidl::TypedefEntity * >(entity.get()))));
    case unoidl::Entity::SORT_CONSTANT_GROUP:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new ConstantGroupDescription(
                    name,
                    static_cast< unoidl::ConstantGroupEntity * >(
                        entity.get()))));
    case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new SingleInterfaceBasedServiceDescription(
                    this, name,
                    static_cast< unoidl::SingleInterfaceBasedServiceEntity * >(
                        entity.get()))));
    case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new AccumulationBasedServiceDescription(
                    this, name,
                    static_cast< unoidl::AccumulationBasedServiceEntity * >(
                        entity.get()))));
    case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new InterfaceBasedSingletonDescription(
                    this, name,
                    static_cast< unoidl::InterfaceBasedSingletonEntity * >(
                        entity.get()))));
    case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new ServiceBasedSingletonDescription(
                    this, name,
                    static_cast< unoidl::ServiceBasedSingletonEntity * >(
                        entity.get()))));
    default:
        for (;;) { std::abort(); } // this cannot happen
    }
}

css::uno::Any cppuhelper::TypeManager::getEnumMember(
    rtl::Reference< unoidl::EnumTypeEntity > const & entity,
    std::u16string_view member)
{
    auto i = std::find_if(entity->getMembers().begin(), entity->getMembers().end(),
        [&member](const unoidl::EnumTypeEntity::Member& rMember) { return rMember.name == member; });
    if (i != entity->getMembers().end())
        return css::uno::Any(i->value);
    return css::uno::Any();
}

css::uno::Any cppuhelper::TypeManager::getConstant(
    std::u16string_view constantGroupName,
    rtl::Reference< unoidl::ConstantGroupEntity > const & entity,
    std::u16string_view member)
{
    auto i = std::find_if(entity->getMembers().begin(), entity->getMembers().end(),
        [&member](const unoidl::ConstantGroupEntity::Member& rMember) { return rMember.name == member; });
    if (i != entity->getMembers().end())
        return css::uno::Any(
            css::uno::Reference< css::reflection::XTypeDescription >(
                new ConstantDescription(OUString(constantGroupName), *i)));
    return css::uno::Any();
}

rtl::Reference< unoidl::Entity > cppuhelper::TypeManager::findEntity(
    OUString const & name)
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
