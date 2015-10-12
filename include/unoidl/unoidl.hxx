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

class LO_DLLPUBLIC_UNOIDL NoSuchFileException {
public:
    SAL_DLLPRIVATE NoSuchFileException(rtl::OUString const & uri): uri_(uri) {}

    SAL_DLLPRIVATE NoSuchFileException(NoSuchFileException const & other):
        uri_(other.uri_) {}

    virtual SAL_DLLPRIVATE ~NoSuchFileException() throw ();

    rtl::OUString getUri() const { return uri_; }

private:
    void operator =(NoSuchFileException) = delete;

    rtl::OUString uri_;
};

class LO_DLLPUBLIC_UNOIDL FileFormatException {
public:
    SAL_DLLPRIVATE FileFormatException(
        rtl::OUString const & uri, rtl::OUString const & detail):
        uri_(uri), detail_(detail)
    {}

    SAL_DLLPRIVATE FileFormatException(FileFormatException const & other):
        uri_(other.uri_), detail_(other.detail_)
    {}

    virtual SAL_DLLPRIVATE ~FileFormatException() throw ();

    rtl::OUString getUri() const { return uri_; }

    rtl::OUString getDetail() const { return detail_; }

private:
    void operator =(FileFormatException) = delete;

    rtl::OUString uri_;
    rtl::OUString detail_;
};

struct AnnotatedReference {
    AnnotatedReference(
        rtl::OUString const & theName,
        std::vector< rtl::OUString > const & theAnnotations):
        name(theName), annotations(theAnnotations)
    {}

    rtl::OUString name;

    std::vector< rtl::OUString > annotations;
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
    virtual rtl::Reference< Entity > getNext(rtl::OUString * name) = 0;

protected:
    SAL_DLLPRIVATE MapCursor() {}

    virtual SAL_DLLPRIVATE ~MapCursor() throw();
};

class LO_DLLPUBLIC_UNOIDL ModuleEntity: public Entity {
public:
    // throws FileFormatException:
    virtual std::vector< rtl::OUString > getMemberNames() const = 0;

    // throws FileFormatException:
    virtual rtl::Reference< MapCursor > createCursor() const = 0;

protected:
    SAL_DLLPRIVATE ModuleEntity(): Entity(SORT_MODULE) {}

    virtual SAL_DLLPRIVATE ~ModuleEntity() throw ();
};

class LO_DLLPUBLIC_UNOIDL PublishableEntity: public Entity {
public:
    bool isPublished() const { return published_; }

    std::vector< rtl::OUString > const & getAnnotations() const
    { return annotations_; }

protected:
    SAL_DLLPRIVATE PublishableEntity(
        Sort sort, bool published,
        std::vector< rtl::OUString > const & annotations):
        Entity(sort), published_(published), annotations_(annotations)
    {}

    virtual SAL_DLLPRIVATE ~PublishableEntity() throw ();

private:
    bool published_;

    std::vector< rtl::OUString > annotations_;
};

class LO_DLLPUBLIC_UNOIDL EnumTypeEntity: public PublishableEntity {
public:
    struct Member {
        Member(
            rtl::OUString const & theName, sal_Int32 theValue,
            std::vector< rtl::OUString > const & theAnnotations):
            name(theName), value(theValue), annotations(theAnnotations)
        {}

        rtl::OUString name;

        sal_Int32 value;

        std::vector< rtl::OUString > annotations;
    };

    SAL_DLLPRIVATE EnumTypeEntity(
        bool published, std::vector< Member > const & members,
        std::vector< rtl::OUString > const & annotations):
        PublishableEntity(SORT_ENUM_TYPE, published, annotations),
        members_(members)
    { assert(!members.empty()); }

    std::vector< Member > const & getMembers() const { return members_; }

private:
    virtual SAL_DLLPRIVATE ~EnumTypeEntity() throw ();

    std::vector< Member > members_;
};

class LO_DLLPUBLIC_UNOIDL PlainStructTypeEntity: public PublishableEntity {
public:
    struct Member {
        Member(rtl::OUString const & theName, rtl::OUString const & theType,
               std::vector< rtl::OUString > const & theAnnotations):
            name(theName), type(theType), annotations(theAnnotations)
        {}

        rtl::OUString name;

        rtl::OUString type;

        std::vector< rtl::OUString > annotations;
    };

    SAL_DLLPRIVATE PlainStructTypeEntity(
        bool published, rtl::OUString const & directBase,
        std::vector< Member > const & directMembers,
        std::vector< rtl::OUString > const & annotations):
        PublishableEntity(SORT_PLAIN_STRUCT_TYPE, published, annotations),
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

class LO_DLLPUBLIC_UNOIDL PolymorphicStructTypeTemplateEntity:
    public PublishableEntity
{
public:
    struct Member {
        Member(
            rtl::OUString const & theName, rtl::OUString const & theType,
            bool theParameterized,
            std::vector< rtl::OUString > const & theAnnotations):
            name(theName), type(theType), parameterized(theParameterized),
            annotations(theAnnotations)
        {}

        rtl::OUString name;

        rtl::OUString type;

        bool parameterized;

        std::vector< rtl::OUString > annotations;
    };

    SAL_DLLPRIVATE PolymorphicStructTypeTemplateEntity(
        bool published, std::vector< rtl::OUString > const & typeParameters,
        std::vector< Member > const & members,
        std::vector< rtl::OUString > const & annotations):
        PublishableEntity(
            SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE, published, annotations),
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

class LO_DLLPUBLIC_UNOIDL ExceptionTypeEntity: public PublishableEntity {
public:
    struct Member {
        Member(
            rtl::OUString const & theName, rtl::OUString const & theType,
            std::vector< rtl::OUString > const & theAnnotations):
            name(theName), type(theType), annotations(theAnnotations)
        {}

        rtl::OUString name;

        rtl::OUString type;

        std::vector< rtl::OUString > annotations;
    };

    SAL_DLLPRIVATE ExceptionTypeEntity(
        bool published, rtl::OUString const & directBase,
        std::vector< Member > const & directMembers,
        std::vector< rtl::OUString > const & annotations):
        PublishableEntity(SORT_EXCEPTION_TYPE, published, annotations),
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

class LO_DLLPUBLIC_UNOIDL InterfaceTypeEntity: public PublishableEntity {
public:
    struct Attribute {
        Attribute(
            rtl::OUString const & theName, rtl::OUString const & theType,
            bool theBound, bool theReadOnly,
            std::vector< rtl::OUString > const & theGetExceptions,
            std::vector< rtl::OUString > const & theSetExceptions,
            std::vector< rtl::OUString > const & theAnnotations):
            name(theName), type(theType), bound(theBound),
            readOnly(theReadOnly), getExceptions(theGetExceptions),
            setExceptions(theSetExceptions), annotations(theAnnotations)
        { assert(!theReadOnly || theSetExceptions.empty()); }

        rtl::OUString name;

        rtl::OUString type;

        bool bound;

        bool readOnly;

        std::vector< rtl::OUString > getExceptions;

        std::vector< rtl::OUString > setExceptions;

        std::vector< rtl::OUString > annotations;
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
            std::vector< rtl::OUString > const & theExceptions,
            std::vector< rtl::OUString > const & theAnnotations):
            name(theName), returnType(theReturnType), parameters(theParameters),
            exceptions(theExceptions), annotations(theAnnotations)
        {}

        rtl::OUString name;

        rtl::OUString returnType;

        std::vector< Parameter > parameters;

        std::vector< rtl::OUString > exceptions;

        std::vector< rtl::OUString > annotations;
    };

    SAL_DLLPRIVATE InterfaceTypeEntity(
        bool published,
        std::vector< AnnotatedReference > const & directMandatoryBases,
        std::vector< AnnotatedReference > const & directOptionalBases,
        std::vector< Attribute > const & directAttributes,
        std::vector< Method > const & directMethods,
        std::vector< rtl::OUString > const & annotations):
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
    virtual SAL_DLLPRIVATE ~InterfaceTypeEntity() throw ();

    std::vector< AnnotatedReference > directMandatoryBases_;
    std::vector< AnnotatedReference > directOptionalBases_;
    std::vector< Attribute > directAttributes_;
    std::vector< Method > directMethods_;
};

class LO_DLLPUBLIC_UNOIDL TypedefEntity: public PublishableEntity {
public:
    SAL_DLLPRIVATE TypedefEntity(
        bool published, rtl::OUString const & type,
        std::vector< rtl::OUString > const & annotations):
        PublishableEntity(SORT_TYPEDEF, published, annotations), type_(type)
    {}

    rtl::OUString getType() const { return type_; }

private:
    virtual SAL_DLLPRIVATE ~TypedefEntity() throw ();

    rtl::OUString type_;
};

struct LO_DLLPUBLIC_UNOIDL ConstantValue {
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

class LO_DLLPUBLIC_UNOIDL ConstantGroupEntity: public PublishableEntity {
public:
    struct Member {
        Member(
            rtl::OUString const & theName, ConstantValue const & theValue,
            std::vector< rtl::OUString > const & theAnnotations):
            name(theName), value(theValue), annotations(theAnnotations)
        {}

        rtl::OUString name;

        ConstantValue value;

        std::vector< rtl::OUString > annotations;
    };

    SAL_DLLPRIVATE ConstantGroupEntity(
        bool published, std::vector< Member > const & members,
        std::vector< rtl::OUString > const & annotations):
        PublishableEntity(SORT_CONSTANT_GROUP, published, annotations),
        members_(members)
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
            std::vector< rtl::OUString > const & theExceptions,
            std::vector< rtl::OUString > const & theAnnotations):
            name(theName), parameters(theParameters), exceptions(theExceptions),
            annotations(theAnnotations), defaultConstructor(false)
        {}

        rtl::OUString name;

        std::vector< Parameter > parameters;

        std::vector< rtl::OUString > exceptions;

        std::vector< rtl::OUString > annotations;

        bool defaultConstructor;
    };

    SAL_DLLPRIVATE SingleInterfaceBasedServiceEntity(
        bool published, rtl::OUString const & base,
        std::vector< Constructor > const & constructors,
        std::vector< rtl::OUString > const & annotations):
        PublishableEntity(
            SORT_SINGLE_INTERFACE_BASED_SERVICE, published, annotations),
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
            rtl::OUString const & theName, rtl::OUString const & theType,
            Attributes theAttributes,
            std::vector< rtl::OUString > const & theAnnotations):
            name(theName), type(theType), attributes(theAttributes),
            annotations(theAnnotations)
        {}

        rtl::OUString name;

        rtl::OUString type;

        Attributes attributes;

        std::vector< rtl::OUString > annotations;
    };

    SAL_DLLPRIVATE AccumulationBasedServiceEntity(
        bool published,
        std::vector< AnnotatedReference > const & directMandatoryBaseServices,
        std::vector< AnnotatedReference > const & directOptionalBaseServices,
        std::vector< AnnotatedReference > const & directMandatoryBaseInterfaces,
        std::vector< AnnotatedReference > const & directOptionalBaseInterfaces,
        std::vector< Property > const & directProperties,
        std::vector< rtl::OUString > const & annotations):
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
    virtual SAL_DLLPRIVATE ~AccumulationBasedServiceEntity() throw ();

    std::vector< AnnotatedReference > directMandatoryBaseServices_;
    std::vector< AnnotatedReference > directOptionalBaseServices_;
    std::vector< AnnotatedReference > directMandatoryBaseInterfaces_;
    std::vector< AnnotatedReference > directOptionalBaseInterfaces_;
    std::vector< Property > directProperties_;
};

class LO_DLLPUBLIC_UNOIDL InterfaceBasedSingletonEntity:
    public PublishableEntity
{
public:
    SAL_DLLPRIVATE InterfaceBasedSingletonEntity(
        bool published, rtl::OUString const & base,
        std::vector< rtl::OUString > const & annotations):
        PublishableEntity(
            SORT_INTERFACE_BASED_SINGLETON, published, annotations),
        base_(base)
    {}

    rtl::OUString getBase() const { return base_; }

private:
    virtual SAL_DLLPRIVATE ~InterfaceBasedSingletonEntity() throw ();

    rtl::OUString base_;
};

class LO_DLLPUBLIC_UNOIDL ServiceBasedSingletonEntity: public PublishableEntity
{
public:
    SAL_DLLPRIVATE ServiceBasedSingletonEntity(
        bool published, rtl::OUString const & base,
        std::vector< rtl::OUString > const & annotations):
        PublishableEntity(SORT_SERVICE_BASED_SINGLETON, published, annotations),
        base_(base)
    {}

    rtl::OUString getBase() const { return base_; }

private:
    virtual SAL_DLLPRIVATE ~ServiceBasedSingletonEntity() throw ();

    rtl::OUString base_;
};

class LO_DLLPUBLIC_UNOIDL Provider: public salhelper::SimpleReferenceObject {
public:
    // throws FileFormatException:
    virtual rtl::Reference< MapCursor > createRootCursor() const = 0;

    // throws FileFormatException:
    virtual rtl::Reference< Entity > findEntity(rtl::OUString const & name)
        const = 0;

protected:
    SAL_DLLPRIVATE Provider() {}

    virtual SAL_DLLPRIVATE ~Provider() throw ();
};

class LO_DLLPUBLIC_UNOIDL Manager: public salhelper::SimpleReferenceObject {
public:
    Manager() {}

    // throws FileFormatException, NoSuchFileException:
    rtl::Reference< Provider > addProvider(rtl::OUString const & uri);

    // throws FileFormatException:
    rtl::Reference< Entity > findEntity(rtl::OUString const & name) const;

    // throws FileFormatException:
    rtl::Reference< MapCursor > createCursor(rtl::OUString const & name) const;

private:
    virtual SAL_DLLPRIVATE ~Manager() throw ();

    SAL_DLLPRIVATE rtl::Reference< Provider > loadProvider(
        rtl::OUString const & uri);

    mutable osl::Mutex mutex_;
    std::vector< rtl::Reference< Provider > > providers_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
