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
    SAL_DLLPRIVATE NoSuchFileException(OUString const & uri): uri_(uri) {}

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
        OUString const & uri, OUString const & detail):
        uri_(uri), detail_(detail)
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
        OUString const & theName,
        std::vector< OUString > const & theAnnotations):
        name(theName), annotations(theAnnotations)
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
        std::vector< OUString > const & annotations):
        Entity(sort), published_(published), annotations_(annotations)
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
            OUString const & theName, sal_Int32 theValue,
            std::vector< OUString > const & theAnnotations):
            name(theName), value(theValue), annotations(theAnnotations)
        {}

        OUString name;

        sal_Int32 value;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE EnumTypeEntity(
        bool published, std::vector< Member > const & members,
        std::vector< OUString > const & annotations):
        PublishableEntity(SORT_ENUM_TYPE, published, annotations),
        members_(members)
    { assert(!members.empty()); }

    std::vector< Member > const & getMembers() const { return members_; }

private:
    virtual SAL_DLLPRIVATE ~EnumTypeEntity() noexcept override;

    std::vector< Member > members_;
};

class SAL_WARN_UNUSED LO_DLLPUBLIC_UNOIDL PlainStructTypeEntity final : public PublishableEntity {
public:
    struct Member {
        Member(OUString const & theName, OUString const & theType,
               std::vector< OUString > const & theAnnotations):
            name(theName), type(theType), annotations(theAnnotations)
        {}

        OUString name;

        OUString type;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE PlainStructTypeEntity(
        bool published, OUString const & directBase,
        std::vector< Member > const & directMembers,
        std::vector< OUString > const & annotations):
        PublishableEntity(SORT_PLAIN_STRUCT_TYPE, published, annotations),
        directBase_(directBase), directMembers_(directMembers)
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
            OUString const & theName, OUString const & theType,
            bool theParameterized,
            std::vector< OUString > const & theAnnotations):
            name(theName), type(theType), parameterized(theParameterized),
            annotations(theAnnotations)
        {}

        OUString name;

        OUString type;

        bool parameterized;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE PolymorphicStructTypeTemplateEntity(
        bool published, std::vector< OUString > const & typeParameters,
        std::vector< Member > const & members,
        std::vector< OUString > const & annotations):
        PublishableEntity(
            SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE, published, annotations),
        typeParameters_(typeParameters), members_(members)
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
            OUString const & theName, OUString const & theType,
            std::vector< OUString > const & theAnnotations):
            name(theName), type(theType), annotations(theAnnotations)
        {}

        OUString name;

        OUString type;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE ExceptionTypeEntity(
        bool published, OUString const & directBase,
        std::vector< Member > const & directMembers,
        std::vector< OUString > const & annotations):
        PublishableEntity(SORT_EXCEPTION_TYPE, published, annotations),
        directBase_(directBase), directMembers_(directMembers)
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
            OUString const & theName, OUString const & theType,
            bool theBound, bool theReadOnly,
            std::vector< OUString > const & theGetExceptions,
            std::vector< OUString > const & theSetExceptions,
            std::vector< OUString > const & theAnnotations):
            name(theName), type(theType), bound(theBound),
            readOnly(theReadOnly), getExceptions(theGetExceptions),
            setExceptions(theSetExceptions), annotations(theAnnotations)
        { assert(!theReadOnly || theSetExceptions.empty()); }

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
            std::vector< OUString > const & theExceptions,
            std::vector< OUString > const & theAnnotations):
            name(theName), returnType(theReturnType), parameters(theParameters),
            exceptions(theExceptions), annotations(theAnnotations)
        {}

        OUString name;

        OUString returnType;

        std::vector< Parameter > parameters;

        std::vector< OUString > exceptions;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE InterfaceTypeEntity(
        bool published,
        std::vector< AnnotatedReference > const & directMandatoryBases,
        std::vector< AnnotatedReference > const & directOptionalBases,
        std::vector< Attribute > const & directAttributes,
        std::vector< Method > const & directMethods,
        std::vector< OUString > const & annotations):
        PublishableEntity(SORT_INTERFACE_TYPE, published, annotations),
        directMandatoryBases_(directMandatoryBases),
        directOptionalBases_(directOptionalBases),
        directAttributes_(directAttributes), directMethods_(directMethods)
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
        bool published, OUString const & type,
        std::vector< OUString > const & annotations):
        PublishableEntity(SORT_TYPEDEF, published, annotations), type_(type)
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
            OUString const & theName, ConstantValue const & theValue,
            std::vector< OUString > const & theAnnotations):
            name(theName), value(theValue), annotations(theAnnotations)
        {}

        OUString name;

        ConstantValue value;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE ConstantGroupEntity(
        bool published, std::vector< Member > const & members,
        std::vector< OUString > const & annotations):
        PublishableEntity(SORT_CONSTANT_GROUP, published, annotations),
        members_(members)
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
                OUString const & theName, OUString const & theType,
                bool theRest):
                name(theName), type(theType), rest(theRest)
            {}

            OUString name;

            OUString type;

            bool rest;
        };

        Constructor():
            defaultConstructor(true) {}

        Constructor(
            OUString const & theName,
            std::vector< Parameter > const & theParameters,
            std::vector< OUString > const & theExceptions,
            std::vector< OUString > const & theAnnotations):
            name(theName), parameters(theParameters), exceptions(theExceptions),
            annotations(theAnnotations), defaultConstructor(false)
        {}

        OUString name;

        std::vector< Parameter > parameters;

        std::vector< OUString > exceptions;

        std::vector< OUString > annotations;

        bool defaultConstructor;
    };

    SAL_DLLPRIVATE SingleInterfaceBasedServiceEntity(
        bool published, OUString const & base,
        std::vector< Constructor > const & constructors,
        std::vector< OUString > const & annotations):
        PublishableEntity(
            SORT_SINGLE_INTERFACE_BASED_SERVICE, published, annotations),
        base_(base), constructors_(constructors)
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
            OUString const & theName, OUString const & theType,
            Attributes theAttributes,
            std::vector< OUString > const & theAnnotations):
            name(theName), type(theType), attributes(theAttributes),
            annotations(theAnnotations)
        {}

        OUString name;

        OUString type;

        Attributes attributes;

        std::vector< OUString > annotations;
    };

    SAL_DLLPRIVATE AccumulationBasedServiceEntity(
        bool published,
        std::vector< AnnotatedReference > const & directMandatoryBaseServices,
        std::vector< AnnotatedReference > const & directOptionalBaseServices,
        std::vector< AnnotatedReference > const & directMandatoryBaseInterfaces,
        std::vector< AnnotatedReference > const & directOptionalBaseInterfaces,
        std::vector< Property > const & directProperties,
        std::vector< OUString > const & annotations):
        PublishableEntity(
            SORT_ACCUMULATION_BASED_SERVICE, published, annotations),
        directMandatoryBaseServices_(directMandatoryBaseServices),
        directOptionalBaseServices_(directOptionalBaseServices),
        directMandatoryBaseInterfaces_(directMandatoryBaseInterfaces),
        directOptionalBaseInterfaces_(directOptionalBaseInterfaces),
        directProperties_(directProperties)
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
        bool published, OUString const & base,
        std::vector< OUString > const & annotations):
        PublishableEntity(
            SORT_INTERFACE_BASED_SINGLETON, published, annotations),
        base_(base)
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
        bool published, OUString const & base,
        std::vector< OUString > const & annotations):
        PublishableEntity(SORT_SERVICE_BASED_SINGLETON, published, annotations),
        base_(base)
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
