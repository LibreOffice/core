/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CPPUHELPER_UNOIDL_HXX
#define INCLUDED_CPPUHELPER_UNOIDL_HXX

#include "sal/config.h"

#include <cassert>
#include <vector>

#include "com/sun/star/uno/Any.hxx"
#include "cppuhelper/cppuhelperdllapi.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"

namespace cppu { namespace unoidl {

class CPPUHELPER_DLLPUBLIC Entity: public salhelper::SimpleReferenceObject {
public:
    enum Sort {
        SORT_MODULE,
        SORT_ENUM_TYPE,
        SORT_PLAIN_STRUCT_TYPE,
        SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE,
        SORT_EXCEPTION_TYPE,
        SORT_INTERFACE_TYPE,
        SORT_TYPEDEF,
        SORT_CONSTANT_GROUP,
        SORT_SINGLE_INTERFACE_BASED_SERVICE,
        SORT_ACCUMULATION_BASED_SERVICE,
        SORT_INTERFACE_BASED_SINGLETON,
        SORT_SERVICE_BASED_SINGLETON
    };

    Sort getSort() const { return sort_; }

protected:
    explicit SAL_DLLPRIVATE Entity(Sort sort): sort_(sort) {}

    virtual SAL_DLLPRIVATE ~Entity() throw ();

private:
    Sort sort_;
};

class CPPUHELPER_DLLPUBLIC MapCursor: public salhelper::SimpleReferenceObject {
public:
    virtual rtl::Reference< Entity > getNext(rtl::OUString * name) = 0;

protected:
    SAL_DLLPRIVATE MapCursor() {}

    virtual SAL_DLLPRIVATE ~MapCursor() throw();
};

class CPPUHELPER_DLLPUBLIC ModuleEntity: public Entity {
public:
    virtual std::vector< rtl::OUString > getMemberNames() const = 0;

    virtual rtl::Reference< MapCursor > createCursor() const = 0;

protected:
    SAL_DLLPRIVATE ModuleEntity(): Entity(SORT_MODULE) {}

    virtual SAL_DLLPRIVATE ~ModuleEntity() throw ();
};

class CPPUHELPER_DLLPUBLIC PublishableEntity: public Entity {
public:
    bool isPublished() const { return published_; }

protected:
    SAL_DLLPRIVATE PublishableEntity(Sort sort, bool published):
        Entity(sort), published_(published)
    {}

    virtual SAL_DLLPRIVATE ~PublishableEntity() throw ();

private:
    bool published_;
};

class CPPUHELPER_DLLPUBLIC EnumTypeEntity: public PublishableEntity {
public:
    struct Member {
        Member(rtl::OUString const & theName, sal_Int32 theValue):
            name(theName), value(theValue)
        {}

        rtl::OUString name;

        sal_Int32 value;
    };

    SAL_DLLPRIVATE EnumTypeEntity(
        bool published, std::vector< Member > const & members):
        PublishableEntity(SORT_ENUM_TYPE, published), members_(members)
    { assert(!members.empty()); }

    std::vector< Member > const & getMembers() const { return members_; }

private:
    virtual SAL_DLLPRIVATE ~EnumTypeEntity() throw ();

    std::vector< Member > members_;
};

class CPPUHELPER_DLLPUBLIC PlainStructTypeEntity: public PublishableEntity {
public:
    struct Member {
        Member(rtl::OUString const & theName, rtl::OUString const & theType):
            name(theName), type(theType)
        {}

        rtl::OUString name;

        rtl::OUString type;
    };

    SAL_DLLPRIVATE PlainStructTypeEntity(
        bool published, rtl::OUString const & directBase,
        std::vector< Member > const & directMembers):
        PublishableEntity(SORT_PLAIN_STRUCT_TYPE, published),
        directBase_(directBase), directMembers_(directMembers)
    {}

    rtl::OUString getDirectBase() const { return directBase_; }

    std::vector< Member > const & getDirectMembers() const
    { return directMembers_; }

private:
    virtual SAL_DLLPRIVATE ~PlainStructTypeEntity() throw ();

    rtl::OUString directBase_;
    std::vector< Member > directMembers_;
};

class CPPUHELPER_DLLPUBLIC PolymorphicStructTypeTemplateEntity:
    public PublishableEntity
{
public:
    struct Member {
        Member(
            rtl::OUString const & theName, rtl::OUString const & theType,
            bool theParameterized):
            name(theName), type(theType), parameterized(theParameterized)
        {}

        rtl::OUString name;

        rtl::OUString type;

        bool parameterized;
    };

    SAL_DLLPRIVATE PolymorphicStructTypeTemplateEntity(
        bool published, std::vector< rtl::OUString > const & typeParameters,
        std::vector< Member > const & members):
        PublishableEntity(SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE, published),
        typeParameters_(typeParameters), members_(members)
    {}

    std::vector< rtl::OUString > const & getTypeParameters() const
    { return typeParameters_; }

    std::vector< Member > const & getMembers() const { return members_; }

private:
    virtual SAL_DLLPRIVATE ~PolymorphicStructTypeTemplateEntity() throw ();

    std::vector< rtl::OUString > typeParameters_;
    std::vector< Member > members_;
};

class CPPUHELPER_DLLPUBLIC ExceptionTypeEntity: public PublishableEntity {
public:
    struct Member {
        Member(rtl::OUString const & theName, rtl::OUString const & theType):
            name(theName), type(theType)
        {}

        rtl::OUString name;

        rtl::OUString type;
    };

    SAL_DLLPRIVATE ExceptionTypeEntity(
        bool published, rtl::OUString const & directBase,
        std::vector< Member > const & directMembers):
        PublishableEntity(SORT_EXCEPTION_TYPE, published),
        directBase_(directBase), directMembers_(directMembers)
    {}

    rtl::OUString getDirectBase() const { return directBase_; }

    std::vector< Member > const & getDirectMembers() const
    { return directMembers_; }

private:
    virtual SAL_DLLPRIVATE ~ExceptionTypeEntity() throw ();

    rtl::OUString directBase_;
    std::vector< Member > directMembers_;
};

class CPPUHELPER_DLLPUBLIC InterfaceTypeEntity: public PublishableEntity {
public:
    struct Attribute {
        Attribute(
            rtl::OUString const & theName, rtl::OUString const & theType,
            bool theBound, bool theReadOnly,
            std::vector< rtl::OUString > const & theGetExceptions,
            std::vector< rtl::OUString > const & theSetExceptions):
            name(theName), type(theType), bound(theBound),
            readOnly(theReadOnly), getExceptions(theGetExceptions),
            setExceptions(theSetExceptions)
        { assert(!theReadOnly || theSetExceptions.empty()); }

        rtl::OUString name;

        rtl::OUString type;

        bool bound;

        bool readOnly;

        std::vector< rtl::OUString > getExceptions;

        std::vector< rtl::OUString > setExceptions;
    };

    struct Method {
        struct Parameter {
            enum Direction { DIRECTION_IN, DIRECTION_OUT, DIRECTION_IN_OUT };

            Parameter(
                rtl::OUString const & theName, rtl::OUString const & theType,
                Direction theDirection):
                name(theName), type(theType), direction(theDirection)
            {}

            rtl::OUString name;

            rtl::OUString type;

            Direction direction;
        };

        Method(
            rtl::OUString const & theName, rtl::OUString const & theReturnType,
            std::vector< Parameter > const & theParameters,
            std::vector< rtl::OUString > const & theExceptions):
            name(theName), returnType(theReturnType), parameters(theParameters),
            exceptions(theExceptions)
        {}

        rtl::OUString name;

        rtl::OUString returnType;

        std::vector< Parameter > parameters;

        std::vector< rtl::OUString > exceptions;
    };

    SAL_DLLPRIVATE InterfaceTypeEntity(
        bool published,
        std::vector< rtl::OUString > const & directMandatoryBases,
        std::vector< rtl::OUString > const & directOptionalBases,
        std::vector< Attribute > const & directAttributes,
        std::vector< Method > const & directMethods):
        PublishableEntity(SORT_INTERFACE_TYPE, published),
        directMandatoryBases_(directMandatoryBases),
        directOptionalBases_(directOptionalBases),
        directAttributes_(directAttributes), directMethods_(directMethods)
    {}

    std::vector< rtl::OUString > const & getDirectMandatoryBases() const
    { return directMandatoryBases_; }

    std::vector< rtl::OUString > const & getDirectOptionalBases() const
    { return directOptionalBases_; }

    std::vector< Attribute > const & getDirectAttributes() const
    { return directAttributes_; }

    std::vector< Method > const & getDirectMethods() const
    { return directMethods_; }

private:
    virtual SAL_DLLPRIVATE ~InterfaceTypeEntity() throw ();

    std::vector< rtl::OUString > directMandatoryBases_;
    std::vector< rtl::OUString > directOptionalBases_;
    std::vector< Attribute > directAttributes_;
    std::vector< Method > directMethods_;
};

class CPPUHELPER_DLLPUBLIC TypedefEntity: public PublishableEntity {
public:
    SAL_DLLPRIVATE TypedefEntity(bool published, rtl::OUString const & type):
        PublishableEntity(SORT_TYPEDEF, published), type_(type)
    {}

    rtl::OUString getType() const { return type_; }

private:
    virtual SAL_DLLPRIVATE ~TypedefEntity() throw ();

    rtl::OUString type_;
};

class CPPUHELPER_DLLPUBLIC ConstantGroupEntity: public PublishableEntity {
public:
    struct Member {
        Member(rtl::OUString const & theName, css::uno::Any const & theValue):
            name(theName), value(theValue)
        {}

        rtl::OUString name;

        css::uno::Any value;
    };

    SAL_DLLPRIVATE ConstantGroupEntity(
        bool published, std::vector< Member > const & members):
        PublishableEntity(SORT_CONSTANT_GROUP, published), members_(members)
    {}

    std::vector< Member > const & getMembers() const { return members_; }

private:
    virtual SAL_DLLPRIVATE ~ConstantGroupEntity() throw ();

    std::vector< Member > members_;
};

class CPPUHELPER_DLLPUBLIC SingleInterfaceBasedServiceEntity:
    public PublishableEntity
{
public:
    struct Constructor {
        struct Parameter {
            Parameter(
                rtl::OUString const & theName, rtl::OUString const & theType,
                bool theRest):
                name(theName), type(theType), rest(theRest)
            {}

            rtl::OUString name;

            rtl::OUString type;

            bool rest;
        };

        Constructor(): defaultConstructor(true) {}

        Constructor(
            rtl::OUString const & theName,
            std::vector< Parameter > const & theParameters,
            std::vector< rtl::OUString > const & theExceptions):
            name(theName), parameters(theParameters), exceptions(theExceptions),
            defaultConstructor(false)
        {}

        rtl::OUString name;

        std::vector< Parameter > parameters;

        std::vector< rtl::OUString > exceptions;

        bool defaultConstructor;
    };

    SAL_DLLPRIVATE SingleInterfaceBasedServiceEntity(
        bool published, rtl::OUString const & base,
        std::vector< Constructor > const & constructors):
        PublishableEntity(SORT_SINGLE_INTERFACE_BASED_SERVICE, published),
        base_(base), constructors_(constructors)
    {}

    rtl::OUString getBase() const { return base_; }

    std::vector< Constructor > const & getConstructors() const
    { return constructors_; }

private:
    virtual SAL_DLLPRIVATE ~SingleInterfaceBasedServiceEntity() throw ();

    rtl::OUString base_;
    std::vector< Constructor > constructors_;
};

class CPPUHELPER_DLLPUBLIC AccumulationBasedServiceEntity:
    public PublishableEntity
{
public:
    struct Property {
        enum Attributes {
            ATTRIBUTE_MAYBE_VOID = 0x001,
            ATTRIBUTE_BOUND = 0x002,
            ATTRIBUTE_CONSTRAINED = 0x004,
            ATTRIBUTE_TRANSIENT = 0x008,
            ATTRIBUTE_READ_ONLY = 0x010,
            ATTRIBUTE_MAYBE_AMBIGUOUS = 0x020,
            ATTRIBUTE_MAYBE_DEFAULT = 0x040,
            ATTRIBUTE_REMOVABLE = 0x080,
            ATTRIBUTE_OPTIONAL = 0x100
        };

        Property(
            rtl::OUString const & theName, rtl::OUString const & theType,
            Attributes theAttributes):
            name(theName), type(theType), attributes(theAttributes)
        {}

        rtl::OUString name;

        rtl::OUString type;

        Attributes attributes;
    };

    SAL_DLLPRIVATE AccumulationBasedServiceEntity(
        bool published,
        std::vector< rtl::OUString > const & directMandatoryBaseServices,
        std::vector< rtl::OUString > const & directOptionalBaseServices,
        std::vector< rtl::OUString > const & directMandatoryBaseInterfaces,
        std::vector< rtl::OUString > const & directOptionalBaseInterfaces,
        std::vector< Property > const & directProperties):
        PublishableEntity(SORT_ACCUMULATION_BASED_SERVICE, published),
        directMandatoryBaseServices_(directMandatoryBaseServices),
        directOptionalBaseServices_(directOptionalBaseServices),
        directMandatoryBaseInterfaces_(directMandatoryBaseInterfaces),
        directOptionalBaseInterfaces_(directOptionalBaseInterfaces),
        directProperties_(directProperties)
        {}

    std::vector< rtl::OUString > const & getDirectMandatoryBaseServices() const
    { return directMandatoryBaseServices_; }

    std::vector< rtl::OUString > const & getDirectOptionalBaseServices() const
    { return directOptionalBaseServices_; }

    std::vector< rtl::OUString > const & getDirectMandatoryBaseInterfaces()
        const
    { return directMandatoryBaseInterfaces_; }

    std::vector< rtl::OUString > const & getDirectOptionalBaseInterfaces() const
    { return directOptionalBaseInterfaces_; }

    std::vector< Property > const & getDirectProperties() const
    { return directProperties_; }

private:
    virtual SAL_DLLPRIVATE ~AccumulationBasedServiceEntity() throw ();

    std::vector< rtl::OUString > directMandatoryBaseServices_;
    std::vector< rtl::OUString > directOptionalBaseServices_;
    std::vector< rtl::OUString > directMandatoryBaseInterfaces_;
    std::vector< rtl::OUString > directOptionalBaseInterfaces_;
    std::vector< Property > directProperties_;
};

class CPPUHELPER_DLLPUBLIC InterfaceBasedSingletonEntity:
    public PublishableEntity
{
public:
    SAL_DLLPRIVATE InterfaceBasedSingletonEntity(
        bool published, rtl::OUString const & base):
        PublishableEntity(SORT_INTERFACE_BASED_SINGLETON, published),
        base_(base)
    {}

    rtl::OUString getBase() const { return base_; }

private:
    virtual SAL_DLLPRIVATE ~InterfaceBasedSingletonEntity() throw ();

    rtl::OUString base_;
};

class CPPUHELPER_DLLPUBLIC ServiceBasedSingletonEntity: public PublishableEntity
{
public:
    SAL_DLLPRIVATE ServiceBasedSingletonEntity(
        bool published, rtl::OUString const & base):
        PublishableEntity(SORT_SERVICE_BASED_SINGLETON, published), base_(base)
    {}

    rtl::OUString getBase() const { return base_; }

private:
    virtual SAL_DLLPRIVATE ~ServiceBasedSingletonEntity() throw ();

    rtl::OUString base_;
};

class CPPUHELPER_DLLPUBLIC Provider: public salhelper::SimpleReferenceObject {
public:
    virtual rtl::Reference< MapCursor > createRootCursor() const = 0;

protected:
    SAL_DLLPRIVATE Provider() {}

    virtual SAL_DLLPRIVATE ~Provider() throw ();
};

CPPUHELPER_DLLPUBLIC rtl::Reference< Provider > loadProvider(
    rtl::OUString const & uri);

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
