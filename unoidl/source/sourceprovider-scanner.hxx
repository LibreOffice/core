/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOIDL_SOURCE_SOURCEPROVIDER_SCANNER_HXX
#define INCLUDED_UNOIDL_SOURCE_SOURCEPROVIDER_SCANNER_HXX

#include "sal/config.h"

#include <cassert>
#include <map>
#include <vector>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"
#include "unoidl/unoidl.hxx"

#include "sourceprovider-parser-requires.hxx"
#include "sourceprovider-parser.hxx"

namespace unoidl { namespace detail {

class SourceProviderEntityPad: public salhelper::SimpleReferenceObject {
public:
    bool isPublished() const { return published_; }

protected:
    explicit SourceProviderEntityPad(bool published): published_(published) {}

    virtual ~SourceProviderEntityPad() {}

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
    virtual ~SourceProviderEnumTypeEntityPad() throw () {}
};

class SourceProviderPlainStructTypeEntityPad: public SourceProviderEntityPad {
public:
    SourceProviderPlainStructTypeEntityPad(
        bool published, OUString theBaseName,
        rtl::Reference<unoidl::PlainStructTypeEntity> const & theBaseEntity):
        SourceProviderEntityPad(published), baseName(theBaseName),
        baseEntity(theBaseEntity)
    { assert(theBaseName.isEmpty() != (bool) theBaseEntity.is()); }

    OUString const baseName;
    rtl::Reference<unoidl::PlainStructTypeEntity> const baseEntity;
    std::vector<unoidl::PlainStructTypeEntity::Member> members;

private:
    virtual ~SourceProviderPlainStructTypeEntityPad() throw () {}
};

class SourceProviderPolymorphicStructTypeTemplateEntityPad:
    public SourceProviderEntityPad
{
public:
    SourceProviderPolymorphicStructTypeTemplateEntityPad(bool published):
        SourceProviderEntityPad(published)
    {}

    std::vector<OUString> typeParameters;
    std::vector<unoidl::PolymorphicStructTypeTemplateEntity::Member> members;

private:
    virtual ~SourceProviderPolymorphicStructTypeTemplateEntityPad() throw () {}
};

class SourceProviderExceptionTypeEntityPad: public SourceProviderEntityPad {
public:
    SourceProviderExceptionTypeEntityPad(
        bool published, OUString theBaseName,
        rtl::Reference<unoidl::ExceptionTypeEntity> const & theBaseEntity):
        SourceProviderEntityPad(published), baseName(theBaseName),
        baseEntity(theBaseEntity)
    { assert(theBaseName.isEmpty() != (bool) theBaseEntity.is()); }

    OUString const baseName;
    rtl::Reference<unoidl::ExceptionTypeEntity> const baseEntity;
    std::vector<unoidl::ExceptionTypeEntity::Member> members;

private:
    virtual ~SourceProviderExceptionTypeEntityPad() throw () {}
};

class SourceProviderInterfaceTypeEntityPad: public SourceProviderEntityPad {
public:
    struct Base {
        Base(
            OUString const & theName,
            rtl::Reference<unoidl::InterfaceTypeEntity> const & theEntity,
            std::vector<OUString> const & theAnnotations):
            name(theName), entity(theEntity), annotations(theAnnotations)
        {}

        OUString name;
        rtl::Reference<unoidl::InterfaceTypeEntity> entity;
        std::vector<OUString> annotations;
    };

    SourceProviderInterfaceTypeEntityPad(
        bool published, OUString singleBaseName,
        rtl::Reference<unoidl::InterfaceTypeEntity> const & singleBaseEntity):
        SourceProviderEntityPad(published),
        singleBase(!singleBaseName.isEmpty())
    {
        assert(singleBaseName.isEmpty() != (bool) singleBaseEntity.is());
        if (singleBase) {
            mandatoryBases.push_back(
                Base(
                    singleBaseName, singleBaseEntity, std::vector<OUString>()));
        }
    }

    bool singleBase;
    std::vector<Base> mandatoryBases;
    std::vector<Base> optionalBases;
    std::vector<unoidl::InterfaceTypeEntity::Attribute> attributes;
    std::vector<unoidl::InterfaceTypeEntity::Method> methods;

private:
    virtual ~SourceProviderInterfaceTypeEntityPad() throw () {}
};

class SourceProviderConstantGroupEntityPad: public SourceProviderEntityPad {
public:
    explicit SourceProviderConstantGroupEntityPad(bool published):
        SourceProviderEntityPad(published)
    {}

    std::vector<unoidl::ConstantGroupEntity::Member> members;

private:
    virtual ~SourceProviderConstantGroupEntityPad() throw () {}
};

class SourceProviderSingleInterfaceBasedServiceEntityPad:
    public SourceProviderEntityPad
{
public:
    struct Constructor {
        struct Parameter {
            Parameter(
                rtl::OUString const & theName,
                SourceProviderType const & theType, bool theRest):
                name(theName), type(theType), rest(theRest)
            {}

            rtl::OUString name;

            SourceProviderType type;

            bool rest;
        };

        Constructor(
            rtl::OUString const & theName,
            std::vector< rtl::OUString > const & theAnnotations):
            name(theName), annotations(theAnnotations)
        {}

        rtl::OUString name;

        std::vector< Parameter > parameters;

        std::vector< rtl::OUString > exceptions;

        std::vector< rtl::OUString > annotations;
    };

    explicit SourceProviderSingleInterfaceBasedServiceEntityPad(
        bool published, OUString const & theBase):
        SourceProviderEntityPad(published), base(theBase)
    {}

    OUString const base;
    std::vector<Constructor> constructors;

private:
    virtual ~SourceProviderSingleInterfaceBasedServiceEntityPad() throw () {}
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
    virtual ~SourceProviderAccumulationBasedServiceEntityPad() throw () {}
};

struct SourceProviderEntity {
    enum Kind { KIND_EXTERNAL, KIND_LOCAL, KIND_INTERFACE_DECL, KIND_MODULE };

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

    SourceProviderEntity() {} // needed for std::map::operator []

    Kind kind;
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<SourceProviderEntityPad> pad;
};

struct SourceProviderScannerData {
    SourceProviderScannerData(
        rtl::Reference<unoidl::Manager> const & theManager):
        manager(theManager), errorLine(0)
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
};

bool parse(OUString const & uri, SourceProviderScannerData * data);

} }

int yylex_init_extra(
    unoidl::detail::SourceProviderScannerData * user_defined,
    yyscan_t * yyscanner);

int yylex_destroy(yyscan_t yyscanner);

int yylex(YYSTYPE * yylval_param, YYLTYPE * yylloc_param, yyscan_t yyscanner);

unoidl::detail::SourceProviderScannerData * yyget_extra(yyscan_t yyscanner);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
