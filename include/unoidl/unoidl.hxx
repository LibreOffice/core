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

#include <sal/config.h>

#include <cassert>
#include <utility>
#include <vector>

#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <salhelper/simplereferenceobject.hxx>
#include <unoidl/detail/dllapi.hxx>

namespace unoidl {

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL NoSuchFileException final {
public:
    SAL_DLLPRIVATE NoSuchFileException(OUString uri): uri_(std::move(uri)) {}

    SAL_DLLPRIVATE NoSuchFileException(NoSuchFileException const & other):
        uri_(other.uri_) {}

    SAL_DLLPRIVATE ~NoSuchFileException() noexcept;

    const OUString& getUri() const { return uri_; }

private:
    NoSuchFileException& operator =(NoSuchFileException const &) = delete;

    OUString uri_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL FileFormatException final {
public:
    SAL_DLLPRIVATE FileFormatException(
        OUString uri, OUString detail):
        uri_(std::move(uri)), detail_(std::move(detail))
    {}

    SAL_DLLPRIVATE FileFormatException(FileFormatException const & other):
        uri_(other.uri_), detail_(other.detail_)
    {}

    SAL_DLLPRIVATE ~FileFormatException() noexcept;

    const OUString& getUri() const { return uri_; }

    const OUString& getDetail() const { return detail_; }

private:
    FileFormatException& operator =(FileFormatException const &) = delete;

    OUString uri_;
    OUString detail_;
};

struct AnnotatedReference {
    AnnotatedReference(
        OUString theName,
        std::vector< OUString > && theAnnotations):
        name(std::move(theName)), annotations(std::move(theAnnotations))
    {}

    OUString name;

    std::vector< OUString > annotations;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL Entity: public salhelper::SimpleReferenceObject {
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

    virtual SAL_DLLPRIVATE ~Entity() noexcept override;

private:
    Sort sort_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL MapCursor: public salhelper::SimpleReferenceObject {
public:
    // throws FileFormatException:
    virtual rtl::Reference< Entity > getNext(OUString * name) = 0;

protected:
    SAL_DLLPRIVATE MapCursor() {}

    virtual SAL_DLLPRIVATE ~MapCursor() noexcept override;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL ModuleEntity: public Entity {
public:
    // throws FileFormatException:
    virtual std::vector< OUString > getMemberNames() const = 0;

    // throws FileFormatException:
    virtual rtl::Reference< MapCursor > createCursor() const = 0;

protected:
    SAL_DLLPRIVATE ModuleEntity(): Entity(SORT_MODULE) {}

    virtual SAL_DLLPRIVATE ~ModuleEntity() noexcept override;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL PublishableEntity: public Entity {
public:
    bool isPublished() const { return published_; }

    std::vector< OUString > const & getAnnotations() const
    { return annotations_; }

protected:
    SAL_DLLPRIVATE PublishableEntity(
        Sort sort, bool published,
        std::vector< OUString >&& annotations):
        Entity(sort), published_(published), annotations_(std::move(annotations))
    {}

    virtual SAL_DLLPRIVATE ~PublishableEntity() noexcept override;

private:
    bool published_;

    std::vector< OUString > annotations_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL EnumTypeEntity final : public PublishableEntity {
public:
    struct Member {
        Member(
            OUString theName, sal_Int32 theValue,
            std::vector< OUString >&& theAnnotations):
            name(std::move(theName)), value(theValue), annotations(std::move(theAnnotations))
        {}

        OUString name;

        sal_Int32 value;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE EnumTypeEntity(
        bool published, std::vector< Member >&& members,
        std::vector< OUString >&& annotations):
        PublishableEntity(SORT_ENUM_TYPE, published, std::move(annotations)),
        members_(std::move(members))
    { assert(!members_.empty()); }

    std::vector< Member > const & getMembers() const { return members_; }

private:
    virtual SAL_DLLPRIVATE ~EnumTypeEntity() noexcept override;

    std::vector< Member > members_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL PlainStructTypeEntity final : public PublishableEntity {
public:
    struct Member {
        Member(OUString theName, OUString theType,
               std::vector< OUString >&& theAnnotations):
            name(std::move(theName)), type(std::move(theType)), annotations(std::move(theAnnotations))
        {}

        OUString name;

        OUString type;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE PlainStructTypeEntity(
        bool published, OUString directBase,
        std::vector< Member >&& directMembers,
        std::vector< OUString > && annotations):
        PublishableEntity(SORT_PLAIN_STRUCT_TYPE, published, std::move(annotations)),
        directBase_(std::move(directBase)), directMembers_(std::move(directMembers))
    {}

    const OUString& getDirectBase() const { return directBase_; }

    std::vector< Member > const & getDirectMembers() const
    { return directMembers_; }

private:
    virtual SAL_DLLPRIVATE ~PlainStructTypeEntity() noexcept override;

    OUString directBase_;
    std::vector< Member > directMembers_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL PolymorphicStructTypeTemplateEntity final :
    public PublishableEntity
{
public:
    struct Member {
        Member(
            OUString theName, OUString theType,
            bool theParameterized,
            std::vector< OUString >&& theAnnotations):
            name(std::move(theName)), type(std::move(theType)), parameterized(theParameterized),
            annotations(std::move(theAnnotations))
        {}

        OUString name;

        OUString type;

        bool parameterized;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE PolymorphicStructTypeTemplateEntity(
        bool published, std::vector< OUString >&& typeParameters,
        std::vector< Member >&& members,
        std::vector< OUString >&& annotations):
        PublishableEntity(
            SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE, published, std::move(annotations)),
        typeParameters_(std::move(typeParameters)), members_(std::move(members))
    {}

    std::vector< OUString > const & getTypeParameters() const
    { return typeParameters_; }

    std::vector< Member > const & getMembers() const { return members_; }

private:
    virtual SAL_DLLPRIVATE ~PolymorphicStructTypeTemplateEntity() noexcept override;

    std::vector< OUString > typeParameters_;
    std::vector< Member > members_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL ExceptionTypeEntity final : public PublishableEntity {
public:
    struct Member {
        Member(
            OUString theName, OUString theType,
            std::vector< OUString >&& theAnnotations):
            name(std::move(theName)), type(std::move(theType)), annotations(std::move(theAnnotations))
        {}

        OUString name;

        OUString type;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE ExceptionTypeEntity(
        bool published, OUString directBase,
        std::vector< Member >&& directMembers,
        std::vector< OUString >&& annotations):
        PublishableEntity(SORT_EXCEPTION_TYPE, published, std::move(annotations)),
        directBase_(std::move(directBase)), directMembers_(std::move(directMembers))
    {}

    const OUString& getDirectBase() const { return directBase_; }

    std::vector< Member > const & getDirectMembers() const
    { return directMembers_; }

private:
    virtual SAL_DLLPRIVATE ~ExceptionTypeEntity() noexcept override;

    OUString directBase_;
    std::vector< Member > directMembers_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL InterfaceTypeEntity final : public PublishableEntity {
public:
    struct Attribute {
        Attribute(
            OUString theName, OUString theType,
            bool theBound, bool theReadOnly,
            std::vector< OUString >&& theGetExceptions,
            std::vector< OUString >&& theSetExceptions,
            std::vector< OUString >&& theAnnotations):
            name(std::move(theName)), type(std::move(theType)), bound(theBound),
            readOnly(theReadOnly), getExceptions(std::move(theGetExceptions)),
            setExceptions(std::move(theSetExceptions)), annotations(std::move(theAnnotations))
        { assert(!theReadOnly || setExceptions.empty()); }

        OUString name;

        OUString type;

        bool bound;

        bool readOnly;

        std::vector< OUString > getExceptions;

        std::vector< OUString > setExceptions;

        std::vector< OUString > annotations;
    };

    struct Method {
        struct Parameter {
            enum Direction { DIRECTION_IN, DIRECTION_OUT, DIRECTION_IN_OUT };

            Parameter(
                OUString theName, OUString theType,
                Direction theDirection):
                name(std::move(theName)), type(std::move(theType)), direction(theDirection)
            {}

            OUString name;

            OUString type;

            Direction direction;
        };

        Method(
            OUString theName, OUString theReturnType,
            std::vector< Parameter >&& theParameters,
            std::vector< OUString >&& theExceptions,
            std::vector< OUString >&& theAnnotations):
            name(std::move(theName)), returnType(std::move(theReturnType)), parameters(std::move(theParameters)),
            exceptions(std::move(theExceptions)), annotations(std::move(theAnnotations))
        {}

        OUString name;

        OUString returnType;

        std::vector< Parameter > parameters;

        std::vector< OUString > exceptions;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE InterfaceTypeEntity(
        bool published,
        std::vector< AnnotatedReference >&& directMandatoryBases,
        std::vector< AnnotatedReference >&& directOptionalBases,
        std::vector< Attribute >&& directAttributes,
        std::vector< Method >&& directMethods,
        std::vector< OUString >&& annotations):
        PublishableEntity(SORT_INTERFACE_TYPE, published, std::move(annotations)),
        directMandatoryBases_(std::move(directMandatoryBases)),
        directOptionalBases_(std::move(directOptionalBases)),
        directAttributes_(std::move(directAttributes)),
        directMethods_(std::move(directMethods))
    {}

    std::vector< AnnotatedReference > const & getDirectMandatoryBases() const
    { return directMandatoryBases_; }

    std::vector< AnnotatedReference > const & getDirectOptionalBases() const
    { return directOptionalBases_; }

    std::vector< Attribute > const & getDirectAttributes() const
    { return directAttributes_; }

    std::vector< Method > const & getDirectMethods() const
    { return directMethods_; }

private:
    virtual SAL_DLLPRIVATE ~InterfaceTypeEntity() noexcept override;

    std::vector< AnnotatedReference > directMandatoryBases_;
    std::vector< AnnotatedReference > directOptionalBases_;
    std::vector< Attribute > directAttributes_;
    std::vector< Method > directMethods_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL TypedefEntity final : public PublishableEntity {
public:
    SAL_DLLPRIVATE TypedefEntity(
        bool published, OUString type,
        std::vector< OUString >&& annotations):
        PublishableEntity(SORT_TYPEDEF, published, std::move(annotations)), type_(std::move(type))
    {}

    const OUString& getType() const { return type_; }

private:
    virtual SAL_DLLPRIVATE ~TypedefEntity() noexcept override;

    OUString type_;
};

struct SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL ConstantValue {
    enum Type {
        TYPE_BOOLEAN, TYPE_BYTE, TYPE_SHORT, TYPE_UNSIGNED_SHORT, TYPE_LONG,
        TYPE_UNSIGNED_LONG, TYPE_HYPER, TYPE_UNSIGNED_HYPER, TYPE_FLOAT,
        TYPE_DOUBLE };

    ConstantValue(bool value): type(TYPE_BOOLEAN), booleanValue(value) {}

    ConstantValue(sal_Int8 value): type(TYPE_BYTE), byteValue(value) {}

    ConstantValue(sal_Int16 value): type(TYPE_SHORT), shortValue(value) {}

    ConstantValue(sal_uInt16 value):
        type(TYPE_UNSIGNED_SHORT), unsignedShortValue(value)
    {}

    ConstantValue(sal_Int32 value): type(TYPE_LONG), longValue(value) {}

    ConstantValue(sal_uInt32 value):
        type(TYPE_UNSIGNED_LONG), unsignedLongValue(value)
    {}

    ConstantValue(sal_Int64 value): type(TYPE_HYPER), hyperValue(value) {}

    ConstantValue(sal_uInt64 value):
        type(TYPE_UNSIGNED_HYPER), unsignedHyperValue(value)
    {}

    ConstantValue(float value): type(TYPE_FLOAT), floatValue(value) {}

    ConstantValue(double value): type(TYPE_DOUBLE), doubleValue(value) {}

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

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL ConstantGroupEntity final : public PublishableEntity {
public:
    struct Member {
        Member(
            OUString theName, ConstantValue const & theValue,
            std::vector< OUString >&& theAnnotations):
            name(std::move(theName)), value(theValue), annotations(std::move(theAnnotations))
        {}

        OUString name;

        ConstantValue value;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE ConstantGroupEntity(
        bool published, std::vector< Member >&& members,
        std::vector< OUString >&& annotations):
        PublishableEntity(SORT_CONSTANT_GROUP, published, std::move(annotations)),
        members_(std::move(members))
    {}

    std::vector< Member > const & getMembers() const { return members_; }

private:
    virtual SAL_DLLPRIVATE ~ConstantGroupEntity() noexcept override;

    std::vector< Member > members_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL SingleInterfaceBasedServiceEntity final :
    public PublishableEntity
{
public:
    struct Constructor {
        struct Parameter {
            Parameter(
                OUString theName, OUString theType,
                bool theRest):
                name(std::move(theName)), type(std::move(theType)), rest(theRest)
            {}

            OUString name;

            OUString type;

            bool rest;
        };

        Constructor():
            defaultConstructor(true) {}

        Constructor(
            OUString theName,
            std::vector< Parameter >&& theParameters,
            std::vector< OUString >&& theExceptions,
            std::vector< OUString >&& theAnnotations):
            name(std::move(theName)), parameters(std::move(theParameters)),
            exceptions(std::move(theExceptions)),
            annotations(std::move(theAnnotations)),
            defaultConstructor(false)
        {}

        OUString name;

        std::vector< Parameter > parameters;

        std::vector< OUString > exceptions;

        std::vector< OUString > annotations;

        bool defaultConstructor;
    };

    SAL_DLLPRIVATE SingleInterfaceBasedServiceEntity(
        bool published, OUString base,
        std::vector< Constructor >&& constructors,
        std::vector< OUString >&& annotations):
        PublishableEntity(
            SORT_SINGLE_INTERFACE_BASED_SERVICE, published, std::move(annotations)),
        base_(std::move(base)), constructors_(std::move(constructors))
    {}

    const OUString& getBase() const { return base_; }

    std::vector< Constructor > const & getConstructors() const
    { return constructors_; }

private:
    virtual SAL_DLLPRIVATE ~SingleInterfaceBasedServiceEntity() noexcept override;

    OUString base_;
    std::vector< Constructor > constructors_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL AccumulationBasedServiceEntity final :
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
            OUString theName, OUString theType,
            Attributes theAttributes,
            std::vector< OUString >&& theAnnotations):
            name(std::move(theName)), type(std::move(theType)), attributes(theAttributes),
            annotations(std::move(theAnnotations))
        {}

        OUString name;

        OUString type;

        Attributes attributes;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE AccumulationBasedServiceEntity(
        bool published,
        std::vector< AnnotatedReference >&& directMandatoryBaseServices,
        std::vector< AnnotatedReference >&& directOptionalBaseServices,
        std::vector< AnnotatedReference >&& directMandatoryBaseInterfaces,
        std::vector< AnnotatedReference >&& directOptionalBaseInterfaces,
        std::vector< Property >&& directProperties,
        std::vector< OUString >&& annotations):
        PublishableEntity(
            SORT_ACCUMULATION_BASED_SERVICE, published, std::move(annotations)),
        directMandatoryBaseServices_(std::move(directMandatoryBaseServices)),
        directOptionalBaseServices_(std::move(directOptionalBaseServices)),
        directMandatoryBaseInterfaces_(std::move(directMandatoryBaseInterfaces)),
        directOptionalBaseInterfaces_(std::move(directOptionalBaseInterfaces)),
        directProperties_(std::move(directProperties))
        {}

    std::vector< AnnotatedReference > const & getDirectMandatoryBaseServices()
        const
    { return directMandatoryBaseServices_; }

    std::vector< AnnotatedReference > const & getDirectOptionalBaseServices()
        const
    { return directOptionalBaseServices_; }

    std::vector< AnnotatedReference > const & getDirectMandatoryBaseInterfaces()
        const
    { return directMandatoryBaseInterfaces_; }

    std::vector< AnnotatedReference > const & getDirectOptionalBaseInterfaces()
        const
    { return directOptionalBaseInterfaces_; }

    std::vector< Property > const & getDirectProperties() const
    { return directProperties_; }

private:
    virtual SAL_DLLPRIVATE ~AccumulationBasedServiceEntity() noexcept override;

    std::vector< AnnotatedReference > directMandatoryBaseServices_;
    std::vector< AnnotatedReference > directOptionalBaseServices_;
    std::vector< AnnotatedReference > directMandatoryBaseInterfaces_;
    std::vector< AnnotatedReference > directOptionalBaseInterfaces_;
    std::vector< Property > directProperties_;
};

class LO_DLLPUBLIC_UNOIDL InterfaceBasedSingletonEntity final :
    public PublishableEntity
{
public:
    SAL_DLLPRIVATE InterfaceBasedSingletonEntity(
        bool published, OUString base,
        std::vector< OUString >&& annotations):
        PublishableEntity(
            SORT_INTERFACE_BASED_SINGLETON, published, std::move(annotations)),
        base_(std::move(base))
    {}

    const OUString& getBase() const { return base_; }

private:
    virtual SAL_DLLPRIVATE ~InterfaceBasedSingletonEntity() noexcept override;

    OUString base_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL ServiceBasedSingletonEntity final : public PublishableEntity
{
public:
    SAL_DLLPRIVATE ServiceBasedSingletonEntity(
        bool published, OUString base,
        std::vector< OUString >&& annotations):
        PublishableEntity(SORT_SERVICE_BASED_SINGLETON, published, std::move(annotations)),
        base_(std::move(base))
    {}

    const OUString& getBase() const { return base_; }

private:
    virtual SAL_DLLPRIVATE ~ServiceBasedSingletonEntity() noexcept override;

    OUString base_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL Provider: public salhelper::SimpleReferenceObject {
public:
    // throws FileFormatException:
    virtual rtl::Reference< MapCursor > createRootCursor() const = 0;

    // throws FileFormatException:
    virtual rtl::Reference< Entity > findEntity(OUString const & name)
        const = 0;

protected:
    SAL_DLLPRIVATE Provider() {}

    virtual SAL_DLLPRIVATE ~Provider() noexcept override;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL Manager final : public salhelper::SimpleReferenceObject {
public:
    Manager() {}

    // throws FileFormatException, NoSuchFileException:
    rtl::Reference< Provider > addProvider(OUString const & uri);

    // throws FileFormatException:
    rtl::Reference< Entity > findEntity(OUString const & name) const;

    // throws FileFormatException:
    rtl::Reference< MapCursor > createCursor(OUString const & name) const;

private:
    virtual SAL_DLLPRIVATE ~Manager() noexcept override;

    SAL_DLLPRIVATE rtl::Reference< Provider > loadProvider(
        OUString const & uri);

    mutable osl::Mutex mutex_;
    std::vector< rtl::Reference< Provider > > providers_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
