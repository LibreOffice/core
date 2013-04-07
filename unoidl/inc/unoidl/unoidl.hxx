/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOIDL_UNOIDL_HXX
#define INCLUDED_UNOIDL_UNOIDL_HXX

#include "sal/config.h"

#include <cassert>
#include <vector>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"
#include "unoidl/detail/dllapi.hxx"

namespace unoidl {

class LO_DLLPUBLIC_UNOIDL NoSuchFileException {
public:
    SAL_DLLPRIVATE NoSuchFileException(OUString const & uri): uri_(uri) {}

    SAL_DLLPRIVATE NoSuchFileException(NoSuchFileException const & other):
        uri_(other.uri_) {}

    virtual SAL_DLLPRIVATE ~NoSuchFileException() throw ();

    OUString getUri() const { return uri_; }

private:
    void operator =(NoSuchFileException) SAL_DELETED_FUNCTION;

    OUString uri_;
};

class LO_DLLPUBLIC_UNOIDL FileFormatException {
public:
    SAL_DLLPRIVATE FileFormatException(
        OUString const & uri, OUString const & detail):
        uri_(uri), detail_(detail)
    {}

    SAL_DLLPRIVATE FileFormatException(FileFormatException const & other):
        uri_(other.uri_), detail_(other.detail_)
    {}

    virtual SAL_DLLPRIVATE ~FileFormatException() throw ();

    OUString getUri() const { return uri_; }

    OUString getDetail() const { return detail_; }

private:
    void operator =(FileFormatException) SAL_DELETED_FUNCTION;

    OUString uri_;
    OUString detail_;
};

class LO_DLLPUBLIC_UNOIDL Entity: public salhelper::SimpleReferenceObject {
public:
    enum Sort {
        SORT_MODULE, SORT_ENUM_TYPE, SORT_PLAIN_STRUCT_TYPE,
        SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE, SORT_EXCEPTION_TYPE,
        SORT_INTERFACE_TYPE, SORT_TYPEDEF, SORT_CONSTANT_GROUP,
        SORT_SINGLE_INTERFACE_BASED_SERVICE, SORT_ACCUMULATION_BASED_SERVICE,
        SORT_INTERFACE_BASED_SINGLETON, SORT_SERVICE_BASED_SINGLETON
    };

    Sort getSort() const { return sort_; }

protected:
    explicit SAL_DLLPRIVATE Entity(Sort sort): sort_(sort) {}

    virtual SAL_DLLPRIVATE ~Entity() throw ();

private:
    Sort sort_;
};

class LO_DLLPUBLIC_UNOIDL MapCursor: public salhelper::SimpleReferenceObject {
public:
    // throws FileFormatException:
    virtual rtl::Reference< Entity > getNext(OUString * name) = 0;

protected:
    SAL_DLLPRIVATE MapCursor() {}

    virtual SAL_DLLPRIVATE ~MapCursor() throw();
};

class LO_DLLPUBLIC_UNOIDL ModuleEntity: public Entity {
public:
    // throws FileFormatException:
    virtual std::vector< OUString > getMemberNames() const = 0;

    // throws FileFormatException:
    virtual rtl::Reference< MapCursor > createCursor() const = 0;

protected:
    SAL_DLLPRIVATE ModuleEntity(): Entity(SORT_MODULE) {}

    virtual SAL_DLLPRIVATE ~ModuleEntity() throw ();
};

class LO_DLLPUBLIC_UNOIDL PublishableEntity: public Entity {
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

class LO_DLLPUBLIC_UNOIDL EnumTypeEntity: public PublishableEntity {
public:
    struct Member {
        Member(OUString const & theName, sal_Int32 theValue):
            name(theName), value(theValue)
        {}

        OUString name;

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

class LO_DLLPUBLIC_UNOIDL PlainStructTypeEntity: public PublishableEntity {
public:
    struct Member {
        Member(OUString const & theName, OUString const & theType):
            name(theName), type(theType)
        {}

        OUString name;

        OUString type;
    };

    SAL_DLLPRIVATE PlainStructTypeEntity(
        bool published, OUString const & directBase,
        std::vector< Member > const & directMembers):
        PublishableEntity(SORT_PLAIN_STRUCT_TYPE, published),
        directBase_(directBase), directMembers_(directMembers)
    {}

    OUString getDirectBase() const { return directBase_; }

    std::vector< Member > const & getDirectMembers() const
    { return directMembers_; }

private:
    virtual SAL_DLLPRIVATE ~PlainStructTypeEntity() throw ();

    OUString directBase_;
    std::vector< Member > directMembers_;
};

class LO_DLLPUBLIC_UNOIDL PolymorphicStructTypeTemplateEntity:
    public PublishableEntity
{
public:
    struct Member {
        Member(
            OUString const & theName, OUString const & theType,
            bool theParameterized):
            name(theName), type(theType), parameterized(theParameterized)
        {}

        OUString name;

        OUString type;

        bool parameterized;
    };

    SAL_DLLPRIVATE PolymorphicStructTypeTemplateEntity(
        bool published, std::vector< OUString > const & typeParameters,
        std::vector< Member > const & members):
        PublishableEntity(SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE, published),
        typeParameters_(typeParameters), members_(members)
    {}

    std::vector< OUString > const & getTypeParameters() const
    { return typeParameters_; }

    std::vector< Member > const & getMembers() const { return members_; }

private:
    virtual SAL_DLLPRIVATE ~PolymorphicStructTypeTemplateEntity() throw ();

    std::vector< OUString > typeParameters_;
    std::vector< Member > members_;
};

class LO_DLLPUBLIC_UNOIDL ExceptionTypeEntity: public PublishableEntity {
public:
    struct Member {
        Member(OUString const & theName, OUString const & theType):
            name(theName), type(theType)
        {}

        OUString name;

        OUString type;
    };

    SAL_DLLPRIVATE ExceptionTypeEntity(
        bool published, OUString const & directBase,
        std::vector< Member > const & directMembers):
        PublishableEntity(SORT_EXCEPTION_TYPE, published),
        directBase_(directBase), directMembers_(directMembers)
    {}

    OUString getDirectBase() const { return directBase_; }

    std::vector< Member > const & getDirectMembers() const
    { return directMembers_; }

private:
    virtual SAL_DLLPRIVATE ~ExceptionTypeEntity() throw ();

    OUString directBase_;
    std::vector< Member > directMembers_;
};

class LO_DLLPUBLIC_UNOIDL InterfaceTypeEntity: public PublishableEntity {
public:
    struct Attribute {
        Attribute(
            OUString const & theName, OUString const & theType,
            bool theBound, bool theReadOnly,
            std::vector< OUString > const & theGetExceptions,
            std::vector< OUString > const & theSetExceptions):
            name(theName), type(theType), bound(theBound),
            readOnly(theReadOnly), getExceptions(theGetExceptions),
            setExceptions(theSetExceptions)
        { assert(!theReadOnly || theSetExceptions.empty()); }

        OUString name;

        OUString type;

        bool bound;

        bool readOnly;

        std::vector< OUString > getExceptions;

        std::vector< OUString > setExceptions;
    };

    struct Method {
        struct Parameter {
            enum Direction { DIRECTION_IN, DIRECTION_OUT, DIRECTION_IN_OUT };

            Parameter(
                OUString const & theName, OUString const & theType,
                Direction theDirection):
                name(theName), type(theType), direction(theDirection)
            {}

            OUString name;

            OUString type;

            Direction direction;
        };

        Method(
            OUString const & theName, OUString const & theReturnType,
            std::vector< Parameter > const & theParameters,
            std::vector< OUString > const & theExceptions):
            name(theName), returnType(theReturnType), parameters(theParameters),
            exceptions(theExceptions)
        {}

        OUString name;

        OUString returnType;

        std::vector< Parameter > parameters;

        std::vector< OUString > exceptions;
    };

    SAL_DLLPRIVATE InterfaceTypeEntity(
        bool published,
        std::vector< OUString > const & directMandatoryBases,
        std::vector< OUString > const & directOptionalBases,
        std::vector< Attribute > const & directAttributes,
        std::vector< Method > const & directMethods):
        PublishableEntity(SORT_INTERFACE_TYPE, published),
        directMandatoryBases_(directMandatoryBases),
        directOptionalBases_(directOptionalBases),
        directAttributes_(directAttributes), directMethods_(directMethods)
    {}

    std::vector< OUString > const & getDirectMandatoryBases() const
    { return directMandatoryBases_; }

    std::vector< OUString > const & getDirectOptionalBases() const
    { return directOptionalBases_; }

    std::vector< Attribute > const & getDirectAttributes() const
    { return directAttributes_; }

    std::vector< Method > const & getDirectMethods() const
    { return directMethods_; }

private:
    virtual SAL_DLLPRIVATE ~InterfaceTypeEntity() throw ();

    std::vector< OUString > directMandatoryBases_;
    std::vector< OUString > directOptionalBases_;
    std::vector< Attribute > directAttributes_;
    std::vector< Method > directMethods_;
};

class LO_DLLPUBLIC_UNOIDL TypedefEntity: public PublishableEntity {
public:
    SAL_DLLPRIVATE TypedefEntity(bool published, OUString const & type):
        PublishableEntity(SORT_TYPEDEF, published), type_(type)
    {}

    OUString getType() const { return type_; }

private:
    virtual SAL_DLLPRIVATE ~TypedefEntity() throw ();

    OUString type_;
};

struct LO_DLLPUBLIC_UNOIDL ConstantValue {
    enum Type {
        TYPE_BOOLEAN, TYPE_BYTE, TYPE_SHORT, TYPE_UNSIGNED_SHORT, TYPE_LONG,
        TYPE_UNSIGNED_LONG, TYPE_HYPER, TYPE_UNSIGNED_HYPER, TYPE_FLOAT,
        TYPE_DOUBLE };

    explicit ConstantValue(bool value): type(TYPE_BOOLEAN), booleanValue(value)
    {}

    explicit ConstantValue(sal_Int8 value): type(TYPE_BYTE), byteValue(value) {}

    explicit ConstantValue(sal_Int16 value): type(TYPE_SHORT), shortValue(value)
    {}

    explicit ConstantValue(sal_uInt16 value):
        type(TYPE_UNSIGNED_SHORT), unsignedShortValue(value)
    {}

    explicit ConstantValue(sal_Int32 value): type(TYPE_LONG), longValue(value)
    {}

    explicit ConstantValue(sal_uInt32 value):
        type(TYPE_UNSIGNED_LONG), unsignedLongValue(value)
    {}

    explicit ConstantValue(sal_Int64 value): type(TYPE_HYPER), hyperValue(value)
    {}

    explicit ConstantValue(sal_uInt64 value):
        type(TYPE_UNSIGNED_HYPER), unsignedHyperValue(value)
    {}

    explicit ConstantValue(float value): type(TYPE_FLOAT), floatValue(value) {}

    explicit ConstantValue(double value): type(TYPE_DOUBLE), doubleValue(value)
    {}

    Type type;

    union {
        bool booleanValue;
        sal_Int8 byteValue;
        sal_Int16 shortValue;
        sal_uInt16 unsignedShortValue;
        sal_Int32 longValue;
        sal_uInt32 unsignedLongValue;
        sal_Int64 hyperValue;
        sal_uInt64 unsignedHyperValue;
        float floatValue;
        double doubleValue;
    };
};

class LO_DLLPUBLIC_UNOIDL ConstantGroupEntity: public PublishableEntity {
public:
    struct Member {
        Member(OUString const & theName, ConstantValue const & theValue):
            name(theName), value(theValue)
        {}

        OUString name;

        ConstantValue value;
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

class LO_DLLPUBLIC_UNOIDL SingleInterfaceBasedServiceEntity:
    public PublishableEntity
{
public:
    struct Constructor {
        struct Parameter {
            Parameter(
                OUString const & theName, OUString const & theType,
                bool theRest):
                name(theName), type(theType), rest(theRest)
            {}

            OUString name;

            OUString type;

            bool rest;
        };

        Constructor(): defaultConstructor(true) {}

        Constructor(
            OUString const & theName,
            std::vector< Parameter > const & theParameters,
            std::vector< OUString > const & theExceptions):
            name(theName), parameters(theParameters), exceptions(theExceptions),
            defaultConstructor(false)
        {}

        OUString name;

        std::vector< Parameter > parameters;

        std::vector< OUString > exceptions;

        bool defaultConstructor;
    };

    SAL_DLLPRIVATE SingleInterfaceBasedServiceEntity(
        bool published, OUString const & base,
        std::vector< Constructor > const & constructors):
        PublishableEntity(SORT_SINGLE_INTERFACE_BASED_SERVICE, published),
        base_(base), constructors_(constructors)
    {}

    OUString getBase() const { return base_; }

    std::vector< Constructor > const & getConstructors() const
    { return constructors_; }

private:
    virtual SAL_DLLPRIVATE ~SingleInterfaceBasedServiceEntity() throw ();

    OUString base_;
    std::vector< Constructor > constructors_;
};

class LO_DLLPUBLIC_UNOIDL AccumulationBasedServiceEntity:
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
            OUString const & theName, OUString const & theType,
            Attributes theAttributes):
            name(theName), type(theType), attributes(theAttributes)
        {}

        OUString name;

        OUString type;

        Attributes attributes;
    };

    SAL_DLLPRIVATE AccumulationBasedServiceEntity(
        bool published,
        std::vector< OUString > const & directMandatoryBaseServices,
        std::vector< OUString > const & directOptionalBaseServices,
        std::vector< OUString > const & directMandatoryBaseInterfaces,
        std::vector< OUString > const & directOptionalBaseInterfaces,
        std::vector< Property > const & directProperties):
        PublishableEntity(SORT_ACCUMULATION_BASED_SERVICE, published),
        directMandatoryBaseServices_(directMandatoryBaseServices),
        directOptionalBaseServices_(directOptionalBaseServices),
        directMandatoryBaseInterfaces_(directMandatoryBaseInterfaces),
        directOptionalBaseInterfaces_(directOptionalBaseInterfaces),
        directProperties_(directProperties)
        {}

    std::vector< OUString > const & getDirectMandatoryBaseServices() const
    { return directMandatoryBaseServices_; }

    std::vector< OUString > const & getDirectOptionalBaseServices() const
    { return directOptionalBaseServices_; }

    std::vector< OUString > const & getDirectMandatoryBaseInterfaces()
        const
    { return directMandatoryBaseInterfaces_; }

    std::vector< OUString > const & getDirectOptionalBaseInterfaces() const
    { return directOptionalBaseInterfaces_; }

    std::vector< Property > const & getDirectProperties() const
    { return directProperties_; }

private:
    virtual SAL_DLLPRIVATE ~AccumulationBasedServiceEntity() throw ();

    std::vector< OUString > directMandatoryBaseServices_;
    std::vector< OUString > directOptionalBaseServices_;
    std::vector< OUString > directMandatoryBaseInterfaces_;
    std::vector< OUString > directOptionalBaseInterfaces_;
    std::vector< Property > directProperties_;
};

class LO_DLLPUBLIC_UNOIDL InterfaceBasedSingletonEntity:
    public PublishableEntity
{
public:
    SAL_DLLPRIVATE InterfaceBasedSingletonEntity(
        bool published, OUString const & base):
        PublishableEntity(SORT_INTERFACE_BASED_SINGLETON, published),
        base_(base)
    {}

    OUString getBase() const { return base_; }

private:
    virtual SAL_DLLPRIVATE ~InterfaceBasedSingletonEntity() throw ();

    OUString base_;
};

class LO_DLLPUBLIC_UNOIDL ServiceBasedSingletonEntity: public PublishableEntity
{
public:
    SAL_DLLPRIVATE ServiceBasedSingletonEntity(
        bool published, OUString const & base):
        PublishableEntity(SORT_SERVICE_BASED_SINGLETON, published), base_(base)
    {}

    OUString getBase() const { return base_; }

private:
    virtual SAL_DLLPRIVATE ~ServiceBasedSingletonEntity() throw ();

    OUString base_;
};

class LO_DLLPUBLIC_UNOIDL Provider: public salhelper::SimpleReferenceObject {
public:
    // throws FileFormatException:
    virtual rtl::Reference< MapCursor > createRootCursor() const = 0;

    // throws FileFormatException:
    virtual rtl::Reference< Entity > findEntity(OUString const & name)
        const = 0;

protected:
    SAL_DLLPRIVATE Provider() {}

    virtual SAL_DLLPRIVATE ~Provider() throw ();
};

class LO_DLLPUBLIC_UNOIDL Manager: public salhelper::SimpleReferenceObject {
public:
    Manager() {}

    void addProvider(rtl::Reference< Provider > const & provider);

    // throws FileFormatException:
    rtl::Reference< Entity > findEntity(OUString const & name) const;

private:
    virtual SAL_DLLPRIVATE ~Manager() throw ();

    std::vector< rtl::Reference< Provider > > providers_;
};

// throws FileFormatException, NoSuchFileException:
LO_DLLPUBLIC_UNOIDL rtl::Reference< Provider > loadProvider(
    rtl::Reference< Manager > const & manager, OUString const & uri);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
