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
#include <cstdlib>
#include <set>
#include <stack>
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
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
#include "com/sun/star/reflection/XTypeDescriptionEnumeration.hpp"
#include "com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/implbase1.hxx"
#include "osl/file.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "unoidl/unoidl.hxx"
#include "unoidl/unoidlprovider.hxx"

#include "paths.hxx"
#include "typedescriptionprovider.hxx"

namespace {

css::uno::Reference< css::reflection::XTypeDescription > resolve(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    rtl::OUString const & name)
{
    assert(context.is());
    try {
        return css::uno::Reference< css::reflection::XTypeDescription >(
            (css::uno::Reference< css::container::XHierarchicalNameAccess >(
                context->getValueByName(
                    "/singletons/"
                    "com.sun.star.reflection.theTypeDescriptionManager"),
                css::uno::UNO_QUERY_THROW)->
             getByHierarchicalName(name)),
            css::uno::UNO_QUERY_THROW);
    } catch (css::container::NoSuchElementException & e) {
        throw css::uno::DeploymentException(
            ("cannot resolve type \"" + name + "\"; NoSuchElementException: "
             + e.Message),
            e.Context);
    }
}

class PublishableDescription:
    public cppu::WeakImplHelper1< css::reflection::XPublished >,
    private boost::noncopyable
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
    public cppu::WeakImplHelper1< css::reflection::XModuleTypeDescription >,
    private boost::noncopyable
{
public:
    ModuleDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::ModuleEntity > const & entity):
        context_(context), name_(name), entity_(entity)
    { assert(entity.is()); }

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

    css::uno::Reference< css::uno::XComponentContext > context_;
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
            s[i] = resolve(context_, name_ + "." + names[i]);
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
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::PlainStructTypeEntity > const & entity):
        PlainStructTypeDescription_Base(entity->isPublished()),
        context_(context), name_(name), entity_(entity)
    { assert(entity.is()); }

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
            : resolve(context_, entity_->getDirectBase());
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

    css::uno::Reference< css::uno::XComponentContext > context_;
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
        s[i] = resolve(context_, entity_->getDirectMembers()[i].type);
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
    public cppu::WeakImplHelper1< css::reflection::XTypeDescription >,
    private boost::noncopyable
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
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > const &
            entity):
        PolymorphicStructTypeTemplateDescription_Base(entity->isPublished()),
        context_(context), name_(name), entity_(entity)
    { assert(entity.is()); }

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

    css::uno::Reference< css::uno::XComponentContext > context_;
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
            : resolve(context_, entity_->getMembers()[i].type);
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

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XCompoundTypeDescription >
ExceptionTypeDescription_Base;

class ExceptionTypeDescription: public ExceptionTypeDescription_Base {
public:
    ExceptionTypeDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::ExceptionTypeEntity > const & entity):
        ExceptionTypeDescription_Base(entity->isPublished()), context_(context),
        name_(name), entity_(entity)
    { assert(entity.is()); }

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
            : resolve(context_, entity_->getDirectBase());
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getMemberNames()
        throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
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
        s[i] = resolve(context_, entity_->getDirectMembers()[i].type);
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

css::uno::Reference< css::reflection::XTypeDescription > resolveTypedefs(
    css::uno::Reference< css::reflection::XTypeDescription > const & type)
{
    css::uno::Reference< css::reflection::XTypeDescription > resolved(type);
    while (resolved->getTypeClass() == css::uno::TypeClass_TYPEDEF) {
        resolved
            = (css::uno::Reference< css::reflection::XIndirectTypeDescription >(
                   resolved, css::uno::UNO_QUERY_THROW)->
               getReferencedType());
    }
    return resolved;
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
                resolveTypedefs(bases[i]), css::uno::UNO_QUERY_THROW));
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

class AttributeDescription:
    public cppu::WeakImplHelper1<
        css::reflection::XInterfaceAttributeTypeDescription2 >,
    private boost::noncopyable
{
public:
    AttributeDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        unoidl::InterfaceTypeEntity::Attribute const & attribute,
        sal_Int32 position):
        context_(context), name_(name), attribute_(attribute),
        position_(position)
    {}

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
    { return resolve(context_, attribute_.type); }

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

    css::uno::Reference< css::uno::XComponentContext > context_;
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
            resolve(context_, attribute_.getExceptions[i]),
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
            resolve(context_, attribute_.setExceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

class MethodParameter:
    public cppu::WeakImplHelper1< css::reflection::XMethodParameter >,
    private boost::noncopyable
{
public:
    MethodParameter(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        unoidl::InterfaceTypeEntity::Method::Parameter const & parameter,
        sal_Int32 position):
        context_(context), parameter_(parameter), position_(position)
    {}

private:
    virtual ~MethodParameter() {}

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return parameter_.name; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() throw (css::uno::RuntimeException)
    { return resolve(context_, parameter_.type); }

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

    css::uno::Reference< css::uno::XComponentContext > context_;
    unoidl::InterfaceTypeEntity::Method::Parameter parameter_;
    sal_Int32 position_;
};

class MethodDescription:
    public cppu::WeakImplHelper1<
        css::reflection::XInterfaceMethodTypeDescription >,
    private boost::noncopyable
{
public:
    MethodDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        unoidl::InterfaceTypeEntity::Method const & method, sal_Int32 position):
        context_(context), name_(name), method_(method), position_(position)
    {}

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
    { return resolve(context_, method_.returnType); }

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

    css::uno::Reference< css::uno::XComponentContext > context_;
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
        s[i] = new MethodParameter(context_, method_.parameters[i], i);
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
        s[i] = resolve(context_, method_.exceptions[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XInterfaceTypeDescription2 >
InterfaceTypeDescription_Base;

class InterfaceTypeDescription: public InterfaceTypeDescription_Base {
public:
    InterfaceTypeDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::InterfaceTypeEntity > const & entity):
        InterfaceTypeDescription_Base(entity->isPublished()), context_(context),
        name_(name), entity_(entity)
    { assert(entity.is()); }

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
            : resolve(context_, entity_->getDirectMandatoryBases()[0]);
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

    css::uno::Reference< css::uno::XComponentContext > context_;
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
            context_, name_ + "::" + entity_->getDirectAttributes()[i].name,
            entity_->getDirectAttributes()[i], off + i);
    }
    for (sal_Int32 i = 0; i != n2; ++i) {
        s[n1 + i] = new MethodDescription(
            context_, name_ + "::" + entity_->getDirectMethods()[i].name,
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
        s[i] = resolve(context_, entity_->getDirectMandatoryBases()[i]);
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
        s[i] = resolve(context_, entity_->getDirectOptionalBases()[i]);
    }
    return s;
}

class ConstantDescription:
    public cppu::WeakImplHelper1< css::reflection::XConstantTypeDescription >,
    private boost::noncopyable
{
public:
    ConstantDescription(
        rtl::OUString const & name, css::uno::Any const & value):
        name_(name), value_(value)
    {}

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

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XConstantsTypeDescription >
ConstantGroupDescription_Base;

class ConstantGroupDescription: public ConstantGroupDescription_Base {
public:
    ConstantGroupDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::ConstantGroupEntity > const & entity):
        ConstantGroupDescription_Base(entity->isPublished()), context_(context),
        name_(name), entity_(entity)
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

    css::uno::Reference< css::uno::XComponentContext > context_;
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
        //TODO: use entity_->getMembers()[i].value directly?
        s[i].set(
            resolve(context_, name_ + "." + entity_->getMembers()[i].name),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XIndirectTypeDescription >
TypedefDescription_Base;

class TypedefDescription: public TypedefDescription_Base {
public:
    TypedefDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::TypedefEntity > const & entity):
        TypedefDescription_Base(entity->isPublished()), context_(context),
        name_(name), entity_(entity)
    { assert(entity.is()); }

private:
    virtual ~TypedefDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_TYPEDEF; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getReferencedType() throw (css::uno::RuntimeException)
    { return resolve(context_, entity_->getType()); }

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    rtl::Reference< unoidl::TypedefEntity > entity_;
};

class ConstructorParameter:
    public cppu::WeakImplHelper1< css::reflection::XParameter >,
    private boost::noncopyable
{
public:
    ConstructorParameter(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter
            const & parameter,
        sal_Int32 position):
        context_(context), parameter_(parameter), position_(position)
    {}

private:
    virtual ~ConstructorParameter() {}

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return parameter_.name; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() throw (css::uno::RuntimeException)
    { return resolve(context_, parameter_.type); }

    virtual sal_Bool SAL_CALL isIn() throw (css::uno::RuntimeException)
    { return true; }

    virtual sal_Bool SAL_CALL isOut() throw (css::uno::RuntimeException)
    { return false; }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return position_; }

    virtual sal_Bool SAL_CALL isRestParameter()
        throw (css::uno::RuntimeException)
    { return parameter_.rest; }

    css::uno::Reference< css::uno::XComponentContext > context_;
    unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter
        parameter_;
    sal_Int32 position_;
};

class ConstructorDescription:
    public cppu::WeakImplHelper1<
        css::reflection::XServiceConstructorDescription >,
    private boost::noncopyable
{
public:
    ConstructorDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        unoidl::SingleInterfaceBasedServiceEntity::Constructor const &
            constructor):
        context_(context), constructor_(constructor)
    {}

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

    css::uno::Reference< css::uno::XComponentContext > context_;
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
            context_, constructor_.parameters[i], i);
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
            resolve(context_, constructor_.exceptions[i]),
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
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > const &
            entity):
        SingleInterfaceBasedServiceDescription_Base(entity->isPublished()),
        context_(context), name_(name), entity_(entity)
    { assert(entity.is()); }

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
    { return resolve(context_, entity_->getBase()); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
    SAL_CALL getConstructors() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
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
            context_, entity_->getConstructors()[i]);
    }
    return s;
}

class PropertyDescription:
    public cppu::WeakImplHelper1< css::reflection::XPropertyTypeDescription >,
    private boost::noncopyable
{
public:
    PropertyDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        unoidl::AccumulationBasedServiceEntity::Property const & property):
        context_(context), property_(property)
    {}

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
    { return resolve(context_, property_.type); }

    css::uno::Reference< css::uno::XComponentContext > context_;
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
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::AccumulationBasedServiceEntity > const &
            entity):
        AccumulationBasedServiceDescription_Base(entity->isPublished()),
        context_(context), name_(name), entity_(entity)
    { assert(entity.is()); }

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

    css::uno::Reference< css::uno::XComponentContext > context_;
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
            resolve(context_, entity_->getDirectMandatoryBaseServices()[i]),
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
            resolve(context_, entity_->getDirectOptionalBaseServices()[i]),
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
                resolve(
                    context_, entity_->getDirectMandatoryBaseInterfaces()[i])),
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
                resolve(
                    context_, entity_->getDirectOptionalBaseInterfaces()[i])),
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
            context_, entity_->getDirectProperties()[i]);
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
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::InterfaceBasedSingletonEntity > const & entity):
        InterfaceBasedSingletonDescription_Base(entity->isPublished()),
        context_(context), name_(name), entity_(entity)
    { assert(entity.is()); }

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
    { return resolve(context_, entity_->getBase()); }

    css::uno::Reference< css::uno::XComponentContext > context_;
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
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        rtl::Reference< unoidl::ServiceBasedSingletonEntity > const & entity):
        ServiceBasedSingletonDescription_Base(entity_->isPublished()),
        context_(context), name_(name), entity_(entity)
    { assert(entity.is()); }

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
            resolve(context_, entity_->getBase()), css::uno::UNO_QUERY_THROW);
    }

    virtual sal_Bool SAL_CALL isInterfaceBased()
        throw (css::uno::RuntimeException)
    { return false; }

    virtual css::uno::Reference< css::reflection::XTypeDescription >
    SAL_CALL getInterface() throw (css::uno::RuntimeException)
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    rtl::Reference< unoidl::ServiceBasedSingletonEntity > entity_;
};

class Enumeration:
    public cppu::WeakImplHelper1<
        css::reflection::XTypeDescriptionEnumeration >,
    private boost::noncopyable
{
public:
    Enumeration(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & prefix,
        rtl::Reference< unoidl::MapCursor > const & cursor,
        css::uno::Sequence< css::uno::TypeClass > const & types, bool deep):
        context_(context), types_(types), deep_(deep)
    {
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

        rtl::OUString prefix;
        rtl::Reference< unoidl::MapCursor > cursor;
        rtl::Reference< unoidl::ConstantGroupEntity > constantGroup;
        std::vector< unoidl::ConstantGroupEntity::Member >::const_iterator
            constantGroupIndex;
    };

    css::uno::Reference< css::uno::XComponentContext > context_;
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
    return resolve(context_, name);
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
                                name + ".",
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
                                name + ".",
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

typedef cppu::WeakComponentImplHelper2<
    css::container::XHierarchicalNameAccess,
    css::reflection::XTypeDescriptionEnumerationAccess >
Provider_Base;

class Provider:
    private osl::Mutex, public Provider_Base, private boost::noncopyable
{
public:
    // throws unoidl::FileFormatException, unoidl::NoSuchFileException:
    Provider(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & uri):
        Provider_Base(*static_cast< osl::Mutex * >(this)), context_(context),
        provider_(new unoidl::UnoidlProvider(uri))
    {}

    using Provider_Base::acquire;
    using Provider_Base::release;

private:
    virtual ~Provider() {}

    virtual void SAL_CALL disposing() {} //TODO

    virtual css::uno::Any SAL_CALL getByHierarchicalName(
        rtl::OUString const & aName)
        throw (
            css::container::NoSuchElementException, css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasByHierarchicalName(
        rtl::OUString const & aName) throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::reflection::XTypeDescriptionEnumeration >
    SAL_CALL createTypeDescriptionEnumeration(
        rtl::OUString const & moduleName,
        css::uno::Sequence< css::uno::TypeClass > const & types,
        css::reflection::TypeDescriptionSearchDepth depth)
        throw(
            css::reflection::NoSuchTypeNameException,
            css::reflection::InvalidTypeNameException,
            css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::Reference< unoidl::UnoidlProvider > provider_;
};

css::uno::Any Provider::getByHierarchicalName(rtl::OUString const & aName)
    throw (css::container::NoSuchElementException, css::uno::RuntimeException)
{
    try {
        bool cnst;
        sal_uInt32 off = provider_->find(aName, &cnst);
        if (off == 0) {
            throw css::container::NoSuchElementException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        if (cnst) {
            unoidl::ConstantValue val1(provider_->getConstant(off));
            css::uno::Any val2;
            switch (val1.type) {
            case unoidl::ConstantValue::TYPE_BOOLEAN:
                val2 <<= val1.booleanValue;
                break;
            case unoidl::ConstantValue::TYPE_BYTE:
                val2 <<= val1.byteValue;
                break;
            case unoidl::ConstantValue::TYPE_SHORT:
                val2 <<= val1.shortValue;
                break;
            case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
                val2 <<= val1.unsignedShortValue;
                break;
            case unoidl::ConstantValue::TYPE_LONG:
                val2 <<= val1.longValue;
                break;
            case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
                val2 <<= val1.unsignedLongValue;
                break;
            case unoidl::ConstantValue::TYPE_HYPER:
                val2 <<= val1.hyperValue;
                break;
            case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
                val2 <<= val1.unsignedHyperValue;
                break;
            case unoidl::ConstantValue::TYPE_FLOAT:
                val2 <<= val1.floatValue;
                break;
            case unoidl::ConstantValue::TYPE_DOUBLE:
                val2 <<= val1.doubleValue;
                break;
            default:
                for (;;) { std::abort(); } // this cannot happen
            }
            return css::uno::makeAny<
                css::uno::Reference< css::reflection::XTypeDescription > >(
                    new ConstantDescription(aName, val2));
        } else {
            rtl::Reference< unoidl::Entity > ent(provider_->getEntity(off));
            switch (ent->getSort()) {
            case unoidl::Entity::SORT_MODULE:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new ModuleDescription(
                            context_, aName,
                            static_cast< unoidl::ModuleEntity * >(ent.get())));
            case unoidl::Entity::SORT_ENUM_TYPE:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new EnumTypeDescription(
                            aName,
                            static_cast< unoidl::EnumTypeEntity * >(
                                ent.get())));
            case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new PlainStructTypeDescription(
                            context_, aName,
                            static_cast< unoidl::PlainStructTypeEntity * >(
                                ent.get())));
            case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new PolymorphicStructTypeTemplateDescription(
                            context_, aName,
                            static_cast<
                                unoidl::PolymorphicStructTypeTemplateEntity * >(
                                    ent.get())));
            case unoidl::Entity::SORT_EXCEPTION_TYPE:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new ExceptionTypeDescription(
                            context_, aName,
                            static_cast< unoidl::ExceptionTypeEntity * >(
                                ent.get())));
            case unoidl::Entity::SORT_INTERFACE_TYPE:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new InterfaceTypeDescription(
                            context_, aName,
                            static_cast< unoidl::InterfaceTypeEntity * >(
                                ent.get())));
            case unoidl::Entity::SORT_TYPEDEF:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new TypedefDescription(
                            context_, aName,
                            static_cast< unoidl::TypedefEntity * >(ent.get())));
            case unoidl::Entity::SORT_CONSTANT_GROUP:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new ConstantGroupDescription(
                            context_, aName,
                            static_cast< unoidl::ConstantGroupEntity * >(
                                ent.get())));
            case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new SingleInterfaceBasedServiceDescription(
                            context_, aName,
                            static_cast<
                                unoidl::SingleInterfaceBasedServiceEntity * >(
                                    ent.get())));
            case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new AccumulationBasedServiceDescription(
                            context_, aName,
                            static_cast<
                                unoidl::AccumulationBasedServiceEntity * >(
                                    ent.get())));
            case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new InterfaceBasedSingletonDescription(
                            context_, aName,
                            static_cast<
                                unoidl::InterfaceBasedSingletonEntity * >(
                                    ent.get())));
            case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new ServiceBasedSingletonDescription(
                            context_, aName,
                            static_cast<
                                unoidl::ServiceBasedSingletonEntity * >(
                                    ent.get())));
            default:
                for (;;) { std::abort(); } // this cannot happen
            }
        }
    } catch (unoidl::FileFormatException & e) {
        throw css::uno::DeploymentException(
            e.getUri() + ": " + e.getDetail(),
            static_cast< cppu::OWeakObject * >(this));
    }
}

sal_Bool Provider::hasByHierarchicalName(
    rtl::OUString const & aName) throw (css::uno::RuntimeException)
{
    try {
        return provider_->find(aName) != 0;
    } catch (unoidl::FileFormatException & e) {
        throw css::uno::DeploymentException(
            e.getUri() + ": " + e.getDetail(),
            static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Reference< css::reflection::XTypeDescriptionEnumeration >
Provider::createTypeDescriptionEnumeration(
    rtl::OUString const & moduleName,
    css::uno::Sequence< css::uno::TypeClass > const & types,
    css::reflection::TypeDescriptionSearchDepth depth)
    throw(
        css::reflection::NoSuchTypeNameException,
        css::reflection::InvalidTypeNameException, css::uno::RuntimeException)
{
    try {
        rtl::OUString prefix;
        rtl::Reference< unoidl::MapCursor > cursor;
        if (moduleName.isEmpty()) {
            cursor = provider_->createRootCursor();
        } else {
            prefix = moduleName + ".";
            bool cnst;
            sal_uInt32 off = provider_->find(moduleName, &cnst);
            if (off == 0) {
                throw css::reflection::NoSuchTypeNameException(
                    moduleName, static_cast< cppu::OWeakObject * >(this));
            }
            if (cnst) {
                throw css::reflection::InvalidTypeNameException(
                    moduleName, static_cast< cppu::OWeakObject * >(this));
            }
            rtl::Reference< unoidl::Entity > ent(provider_->getEntity(off));
            if (ent->getSort() != unoidl::Entity::SORT_MODULE) {
                throw css::reflection::InvalidTypeNameException(
                    moduleName, static_cast< cppu::OWeakObject * >(this));
            }
            cursor = static_cast< unoidl::ModuleEntity * >(ent.get())->
                createCursor();
        }
        return new Enumeration(
            context_, prefix, cursor, types,
            depth == css::reflection::TypeDescriptionSearchDepth_INFINITE);
    } catch (unoidl::FileFormatException & e) {
        throw css::uno::DeploymentException(
            e.getUri() + ": " + e.getDetail(),
            static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
readLegacyRdbFile(
    rtl::OUString const & uri,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    assert(serviceManager.is());
    css::uno::Reference< css::registry::XSimpleRegistry > reg(
        serviceManager->createInstanceWithContext(
            "com.sun.star.comp.stoc.SimpleRegistry", context),
        css::uno::UNO_QUERY_THROW);
    try {
        reg->open(uri, true, false);
    } catch (css::registry::InvalidRegistryException & e) {
        throw css::uno::DeploymentException(
            "Invalid registry " + uri + ":" + e.Message,
            css::uno::Reference< css::uno::XInterface >());
    }
    css::uno::Sequence< css::uno::Any > arg(1);
    arg[0] <<= reg;
    return css::uno::Reference< css::container::XHierarchicalNameAccess >(
        serviceManager->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.stoc.RegistryTypeDescriptionProvider", arg,
            context),
        css::uno::UNO_QUERY_THROW);
}

void readRdbFile(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context,
    std::vector<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > *
            providers)
{
    assert(providers != 0);
    css::uno::Reference< css::container::XHierarchicalNameAccess > prov;
    try {
        prov = new Provider(context, uri);
    } catch (unoidl::NoSuchFileException &) {
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << uri);
            return;
        }
        throw css::uno::DeploymentException(
            uri + ": no such file",
            css::uno::Reference< css::uno::XInterface >());
    } catch (unoidl::FileFormatException &) {
        prov = readLegacyRdbFile(uri, serviceManager, context);
    }
    assert(prov.is());
    providers->push_back(prov);
}

void readRdbDirectory(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context,
    std::vector<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > *
            providers)
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
            css::uno::Reference< css::uno::XInterface >());
    }
    for (;;) {
        rtl::OUString fileUri;
        if (!cppu::nextDirectoryItem(dir, &fileUri)) {
            break;
        }
        readRdbFile(fileUri, optional, serviceManager, context, providers);
    }
}

}

css::uno::Sequence<
    css::uno::Reference< css::container::XHierarchicalNameAccess > >
cppuhelper::createTypeDescriptionProviders(
    rtl::OUString const & uris,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    std::vector<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > provs;
    for (sal_Int32 i = 0; i != -1;) {
        rtl::OUString uri(uris.getToken(0, ' ', i));
        if (uri.isEmpty()) {
            continue;
        }
        bool optional;
        bool directory;
        cppu::decodeRdbUri(&uri, &optional, &directory);
        if (directory) {
            readRdbDirectory(uri, optional, serviceManager, context, &provs);
        } else {
            readRdbFile(uri, optional, serviceManager, context, &provs);
        }
    }
    css::uno::Sequence<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > provs2(
            static_cast< sal_Int32 >(provs.size())); //TODO: check overflow
    std::vector<
        css::uno::Reference<
            css::container::XHierarchicalNameAccess > >::iterator i(
                provs.begin());
    for (sal_Int32 j = 0; j != provs2.getLength(); ++j) {
        provs2[j] = *i++;
    }
    return provs2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
