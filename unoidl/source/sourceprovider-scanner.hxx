/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <cassert>
#include <map>
#include <set>
#include <vector>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <salhelper/simplereferenceobject.hxx>
#include <unoidl/unoidl.hxx>

#include "sourceprovider-parser-requires.hxx"
#include <sourceprovider-parser.hxx>

namespace unoidl::detail {

struct SourceProviderScannerData;

class SourceProviderEntityPad: public salhelper::SimpleReferenceObject {
public:
    bool isPublished() const { return published_; }

protected:
    explicit SourceProviderEntityPad(bool published): published_(published) {}

    virtual ~SourceProviderEntityPad() override {}

private:
    bool const published_;
};

class SourceProviderEnumTypeEntityPad: public SourceProviderEntityPad {
public:
    explicit SourceProviderEnumTypeEntityPad(bool published):
        SourceProviderEntityPad(published)
    {}

    std::vector<unoidl::EnumTypeEntity::Member> members;

private:
    virtual ~SourceProviderEnumTypeEntityPad() noexcept override {}
};

class SourceProviderPlainStructTypeEntityPad: public SourceProviderEntityPad {
public:
    SourceProviderPlainStructTypeEntityPad(
        bool published, const OUString & theBaseName,
        rtl::Reference<unoidl::PlainStructTypeEntity> const & theBaseEntity):
        SourceProviderEntityPad(published), baseName(theBaseName),
        baseEntity(theBaseEntity)
    { assert(theBaseName.isEmpty() != theBaseEntity.is()); }

    OUString const baseName;
    rtl::Reference<unoidl::PlainStructTypeEntity> const baseEntity;
    std::vector<unoidl::PlainStructTypeEntity::Member> members;

private:
    virtual ~SourceProviderPlainStructTypeEntityPad() noexcept override {}
};

class SourceProviderPolymorphicStructTypeTemplateEntityPad:
    public SourceProviderEntityPad
{
public:
    explicit SourceProviderPolymorphicStructTypeTemplateEntityPad(bool published)
        : SourceProviderEntityPad(published)
    {}

    std::vector<OUString> typeParameters;
    std::vector<unoidl::PolymorphicStructTypeTemplateEntity::Member> members;

private:
    virtual ~SourceProviderPolymorphicStructTypeTemplateEntityPad() noexcept override {}
};

class SourceProviderExceptionTypeEntityPad: public SourceProviderEntityPad {
public:
    SourceProviderExceptionTypeEntityPad(
        bool published, const OUString & theBaseName,
        rtl::Reference<unoidl::ExceptionTypeEntity> const & theBaseEntity):
        SourceProviderEntityPad(published), baseName(theBaseName),
        baseEntity(theBaseEntity)
    { assert(theBaseName.isEmpty() != theBaseEntity.is()); }

    OUString const baseName;
    rtl::Reference<unoidl::ExceptionTypeEntity> const baseEntity;
    std::vector<unoidl::ExceptionTypeEntity::Member> members;

private:
    virtual ~SourceProviderExceptionTypeEntityPad() noexcept override {}
};

class SourceProviderInterfaceTypeEntityPad: public SourceProviderEntityPad {
public:
    struct DirectBase {
        DirectBase(
            OUString const & theName,
            rtl::Reference<unoidl::InterfaceTypeEntity> const & theEntity,
            std::vector<OUString> const & theAnnotations):
            name(theName), entity(theEntity), annotations(theAnnotations)
        { assert(theEntity.is()); }

        OUString name;
        rtl::Reference<unoidl::InterfaceTypeEntity> entity;
        std::vector<OUString> annotations;
    };

    enum BaseKind {
        BASE_INDIRECT_OPTIONAL, BASE_DIRECT_OPTIONAL, BASE_INDIRECT_MANDATORY,
        BASE_DIRECT_MANDATORY
    };

    struct Member {
        OUString mandatory;
        std::set<OUString> optional;

        explicit Member(const OUString & theMandatory): mandatory(theMandatory) {}
    };

    SourceProviderInterfaceTypeEntityPad(bool published, bool theSingleBase):
        SourceProviderEntityPad(published), singleBase(theSingleBase)
    {}

    bool addDirectBase(
        YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
        DirectBase const & base, bool optional);

    bool addDirectMember(
        YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
        OUString const & name);

    bool singleBase;
    std::vector<DirectBase> directMandatoryBases;
    std::vector<DirectBase> directOptionalBases;
    std::vector<unoidl::InterfaceTypeEntity::Attribute> directAttributes;
    std::vector<unoidl::InterfaceTypeEntity::Method> directMethods;
    std::map<OUString, BaseKind> allBases;
    std::map<OUString, Member> allMembers;

private:
    virtual ~SourceProviderInterfaceTypeEntityPad() noexcept override {}

    bool checkBaseClashes(
        YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
        OUString const & name,
        rtl::Reference<unoidl::InterfaceTypeEntity> const & entity,
        bool direct, bool optional, bool outerOptional,
        std::set<OUString> * seen) const;

    bool checkMemberClashes(
        YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
        std::u16string_view interfaceName, OUString const & memberName,
        bool checkOptional) const;

    bool addBase(
        YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
        OUString const & directBaseName, OUString const & name,
        rtl::Reference<unoidl::InterfaceTypeEntity> const & entity, bool direct,
        bool optional);

    bool addOptionalBaseMembers(
        YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
        OUString const & name,
        rtl::Reference<unoidl::InterfaceTypeEntity> const & entity);
};

class SourceProviderConstantGroupEntityPad: public SourceProviderEntityPad {
public:
    explicit SourceProviderConstantGroupEntityPad(bool published):
        SourceProviderEntityPad(published)
    {}

    std::vector<unoidl::ConstantGroupEntity::Member> members;

private:
    virtual ~SourceProviderConstantGroupEntityPad() noexcept override {}
};

class SourceProviderSingleInterfaceBasedServiceEntityPad:
    public SourceProviderEntityPad
{
public:
    struct Constructor {
        struct Parameter {
            Parameter(
                OUString const & theName,
                SourceProviderType const & theType, bool theRest):
                name(theName), type(theType), rest(theRest)
            {}

            OUString name;

            SourceProviderType type;

            bool rest;
        };

        Constructor(
            OUString const & theName,
            std::vector< OUString > const & theAnnotations):
            name(theName), annotations(theAnnotations)
        {}

        OUString name;

        std::vector< Parameter > parameters;

        std::vector< OUString > exceptions;

        std::vector< OUString > annotations;
    };

    explicit SourceProviderSingleInterfaceBasedServiceEntityPad(
        bool published, OUString const & theBase):
        SourceProviderEntityPad(published), base(theBase)
    {}

    OUString const base;
    std::vector<Constructor> constructors;

private:
    virtual ~SourceProviderSingleInterfaceBasedServiceEntityPad() noexcept override {}
};

class SourceProviderAccumulationBasedServiceEntityPad:
    public SourceProviderEntityPad
{
public:
    explicit SourceProviderAccumulationBasedServiceEntityPad(bool published):
        SourceProviderEntityPad(published)
    {}

    std::vector<unoidl::AnnotatedReference> directMandatoryBaseServices;
    std::vector<unoidl::AnnotatedReference> directOptionalBaseServices;
    std::vector<unoidl::AnnotatedReference> directMandatoryBaseInterfaces;
    std::vector<unoidl::AnnotatedReference> directOptionalBaseInterfaces;
    std::vector<unoidl::AccumulationBasedServiceEntity::Property>
        directProperties;

private:
    virtual ~SourceProviderAccumulationBasedServiceEntityPad() noexcept override {}
};

struct SourceProviderEntity {
    enum Kind {
        KIND_EXTERNAL, KIND_LOCAL, KIND_INTERFACE_DECL,
        KIND_PUBLISHED_INTERFACE_DECL, KIND_MODULE
    };

    explicit SourceProviderEntity(
        Kind theKind, rtl::Reference<unoidl::Entity> const & externalEntity):
        kind(theKind), entity(externalEntity)
    { assert(theKind <= KIND_LOCAL); assert(externalEntity.is()); }

    explicit SourceProviderEntity(
        rtl::Reference<SourceProviderEntityPad> const & localPad):
        kind(KIND_LOCAL), pad(localPad)
    { assert(localPad.is()); }

    explicit SourceProviderEntity(Kind theKind): kind(theKind)
    { assert(theKind >= KIND_INTERFACE_DECL); }

    SourceProviderEntity(): // needed for std::map::operator []
        kind() // avoid false warnings about uninitialized members
    {}

    Kind kind;
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<SourceProviderEntityPad> pad;
};

struct SourceProviderScannerData {
    explicit SourceProviderScannerData(
        rtl::Reference<unoidl::Manager> const & theManager):
        manager(theManager),
        sourcePosition(), sourceEnd(),
            // avoid false warnings about uninitialized members
        errorLine(0), publishedContext(false)
    { assert(manager.is()); }

    void setSource(void const * address, sal_uInt64 size) {
        sourcePosition = static_cast<char const *>(address);
        sourceEnd = sourcePosition + size;
    }

    rtl::Reference<unoidl::Manager> manager;

    char const * sourcePosition;
    char const * sourceEnd;
    YYLTYPE errorLine;
    OString parserError;
    OUString errorMessage;

    std::map<OUString, SourceProviderEntity> entities;
    std::vector<OUString> modules;
    OUString currentName;
    bool publishedContext;
};

bool parse(OUString const & uri, SourceProviderScannerData * data);

}

int yylex_init_extra(
    unoidl::detail::SourceProviderScannerData * user_defined,
    yyscan_t * yyscanner);

int yylex_destroy(yyscan_t yyscanner);

int yylex(YYSTYPE * yylval_param, YYLTYPE * yylloc_param, yyscan_t yyscanner);

unoidl::detail::SourceProviderScannerData * yyget_extra(yyscan_t yyscanner);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
