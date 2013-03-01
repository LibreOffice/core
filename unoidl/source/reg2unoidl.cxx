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
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "osl/endian.h"
#include "osl/file.h"
#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/process.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/textcvt.h"
#include "rtl/ustring.hxx"
#include "sal/macros.h"
#include "sal/main.h"
#include "unoidl/unoidl.hxx"

namespace {

OUString getArgumentUri(sal_uInt32 argument) {
    OUString arg;
    rtl_getAppCommandArg(argument, &arg.pData);
    OUString url;
    osl::FileBase::RC e1 = osl::FileBase::getFileURLFromSystemPath(arg, url);
    if (e1 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot convert \"" << arg << "\" to file URL, error code "
            << +e1 << std::endl;
        std::exit(EXIT_FAILURE);
    }
    OUString cwd;
    oslProcessError e2 = osl_getProcessWorkingDir(&cwd.pData);
    if (e2 != osl_Process_E_None) {
        std::cerr
            << "Cannot obtain working directory, error code " << +e2
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
    OUString abs;
    e1 = osl::FileBase::getAbsoluteFileURL(cwd, url, abs);
    if (e1 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot make \"" << url
            << "\" into an absolute file URL, error code " << +e1 << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return abs;
}

rtl::Reference< unoidl::Provider > load(
    rtl::Reference< unoidl::Manager > const & manager, OUString const & uri)
{
    try {
        return unoidl::loadProvider(manager, uri);
    } catch (unoidl::NoSuchFileException &) {
        std::cerr << "Input <" << uri << "> does not exist" << std::endl;
        std::exit(EXIT_FAILURE);
    } catch (unoidl::FileFormatException & e) {
        std::cerr
            << "Cannot read input <" << uri << ">: " << e.getDetail()
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

sal_uInt64 getOffset(osl::File & file) {
    sal_uInt64 off;
    osl::FileBase::RC e = file.getPos(off);
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot determine current position in <" << file.getURL()
            << ">, error code " << +e << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return off;
}

void write(osl::File & file, void const * buffer, sal_uInt64 size) {
    sal_uInt64 n;
    osl::FileBase::RC e = file.write(buffer, size, n);
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot write to <" << file.getURL() << ">, error code " << +e
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (n != size) {
        std::cerr
            << "Bad write of " << n << " instead of " << size << " bytes to <"
            << file.getURL() << '>' << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void write8(osl::File & file, sal_uInt64 value) {
    if (value > 0xFF) {
        std::cerr
            << "Cannot write value >= 2^8; input is too large" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    unsigned char buf[1];
    buf[0] = value & 0xFF;
    write(file, buf, SAL_N_ELEMENTS(buf));
}

void write16(osl::File & file, sal_uInt64 value) {
    if (value > 0xFFFF) {
        std::cerr
            << "Cannot write value >= 2^16; input is too large" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    unsigned char buf[2];
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    write(file, buf, SAL_N_ELEMENTS(buf));
}

void write32(osl::File & file, sal_uInt64 value) {
    if (value > 0xFFFFFFFF) {
        std::cerr
            << "Cannot write value >= 2^32; input is too large" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    unsigned char buf[4];
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
    write(file, buf, SAL_N_ELEMENTS(buf));
}

void write64(osl::File & file, sal_uInt64 value) {
    unsigned char buf[8];
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
    buf[3] = (value >> 32) & 0xFF;
    buf[3] = (value >> 40) & 0xFF;
    buf[3] = (value >> 48) & 0xFF;
    buf[3] = (value >> 56) & 0xFF;
    write(file, buf, SAL_N_ELEMENTS(buf));
}

void writeIso60599Binary32(osl::File & file, float value) {
    union {
        unsigned char buf[4];
        float f; // assuming float is ISO 60599 binary32
    } sa;
    sa.f = value;
#if defined OSL_BIGENDIAN
    std::swap(sa.buf[0], sa.buf[3]);
    std::swap(sa.buf[1], sa.buf[2]);
#endif
    write(file, sa.buf, SAL_N_ELEMENTS(sa.buf));
}

void writeIso60599Binary64(osl::File & file, double value) {
    union {
        unsigned char buf[8];
        float d; // assuming double is ISO 60599 binary64
    } sa;
    sa.d = value;
#if defined OSL_BIGENDIAN
    std::swap(sa.buf[0], sa.buf[7]);
    std::swap(sa.buf[1], sa.buf[6]);
    std::swap(sa.buf[2], sa.buf[5]);
    std::swap(sa.buf[3], sa.buf[4]);
#endif
    write(file, sa.buf, SAL_N_ELEMENTS(sa.buf));
}

OString toAscii(OUString const & name) {
    OString ascii;
    if (!name.convertToString(
            &ascii, RTL_TEXTENCODING_ASCII_US,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        std::cerr
            << "Cannot convert \"" << name << "\" to US ASCII" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return ascii;
}

sal_uInt64 writeNameNul(osl::File & file, OUString const & name) {
    OString ascii(toAscii(name));
    if (ascii.indexOf('\0') != -1) {
        std::cerr
            << "Name \"" << ascii << "\" contains NUL characters" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    sal_uInt64 off = getOffset(file);
    write(file, ascii.getStr(), ascii.getLength() + 1);
    return off;
}

void writeNameLen(osl::File & file, OUString const & name) {
    static std::map< OUString, sal_uInt64 > reuse;
    std::map< OUString, sal_uInt64 >::iterator i(reuse.find(name));
    if (i == reuse.end()) {
        reuse.insert(std::make_pair(name, getOffset(file)));
        OString ascii(toAscii(name));
        assert(
            (static_cast< sal_uInt64 >(ascii.getLength()) & 0x80000000) == 0);
        write32(
            file, static_cast< sal_uInt64 >(ascii.getLength()) | 0x80000000);
        write(file, ascii.getStr(), ascii.getLength());
    } else {
        write32(file, i->second);
    }
}

void writeKind(
    osl::File & file,
    rtl::Reference< unoidl::PublishableEntity > const & entity,
    bool flag = false)
{
    assert(entity.is());
    sal_uInt64 v = entity->getSort();
    if (entity->isPublished()) {
        v |= 0x80;
    }
    if (false /*TODO: deprecated */) {
        v |= 0x40;
    }
    if (flag) {
        v |= 0x20;
    }
    write8(file, v);
}

struct Item {
    explicit Item(rtl::Reference< unoidl::Entity > const & theEntity):
        entity(theEntity)
    {}

    rtl::Reference< unoidl::Entity > entity;
    sal_uInt64 nameOffset;
    sal_uInt64 dataOffset;
};

struct ConstItem {
    explicit ConstItem(unoidl::ConstantValue const & theConstant):
        constant(theConstant)
    {}

    unoidl::ConstantValue constant;
    sal_uInt64 nameOffset;
    sal_uInt64 dataOffset;
};

sal_uInt64 writeMap(
    osl::File & file, rtl::Reference< unoidl::MapCursor > const & cursor,
    std::size_t * rootSize)
{
    assert(cursor.is());
    std::map< OUString, Item > map;
    for (;;) {
        OUString name;
        rtl::Reference< unoidl::Entity > ent(cursor->getNext(&name));
        if (!ent.is()) {
            break;
        }
        if (!map.insert(std::make_pair(name, Item(ent))).second) {
            std::cout << "Duplicate name \"" << name << '"' << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    for (std::map< OUString, Item >::iterator i(map.begin()); i != map.end();
         ++i)
    {
        switch (i->second.entity->getSort()) {
        case unoidl::Entity::SORT_MODULE:
            {
                rtl::Reference< unoidl::ModuleEntity > ent2(
                    static_cast< unoidl::ModuleEntity * >(
                        i->second.entity.get()));
                i->second.dataOffset = writeMap(file, ent2->createCursor(), 0);
                break;
            }
        case unoidl::Entity::SORT_ENUM_TYPE:
            {
                rtl::Reference< unoidl::EnumTypeEntity > ent2(
                    static_cast< unoidl::EnumTypeEntity * >(
                        i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get());
                write32(file, ent2->getMembers().size());
                for (std::vector< unoidl::EnumTypeEntity::Member >::
                         const_iterator j(ent2->getMembers().begin());
                     j != ent2->getMembers().end(); ++j)
                {
                    writeNameLen(file, j->name);
                    write32(file, static_cast< sal_uInt32 >(j->value));
                }
                break;
            }
        case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
            {
                rtl::Reference< unoidl::PlainStructTypeEntity > ent2(
                    static_cast< unoidl::PlainStructTypeEntity * >(
                        i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), !ent2->getDirectBase().isEmpty());
                if (!ent2->getDirectBase().isEmpty()) {
                    writeNameLen(file, ent2->getDirectBase());
                }
                write32(file, ent2->getDirectMembers().size());
                for (std::vector< unoidl::PlainStructTypeEntity::Member >::
                         const_iterator j(ent2->getDirectMembers().begin());
                     j != ent2->getDirectMembers().end(); ++j)
                {
                    writeNameLen(file, j->name);
                    writeNameLen(file, j->type);
                }
                break;
            }
        case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
            {
                rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity >
                    ent2(
                        static_cast<
                        unoidl::PolymorphicStructTypeTemplateEntity * >(
                            i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get());
                write32(file, ent2->getTypeParameters().size());
                for (std::vector< OUString >::const_iterator j(
                         ent2->getTypeParameters().begin());
                     j != ent2->getTypeParameters().end(); ++j)
                {
                    writeNameLen(file, *j);
                }
                write32(file, ent2->getMembers().size());
                for (std::vector<
                         unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                         const_iterator j(
                             ent2->getMembers().begin());
                     j != ent2->getMembers().end(); ++j)
                {
                    sal_uInt64 f = 0;
                    if (j->parameterized) {
                        f |= 0x01;
                    }
                    write8(file, f);
                    writeNameLen(file, j->name);
                    writeNameLen(file, j->type);
                }
                break;
            }
        case unoidl::Entity::SORT_EXCEPTION_TYPE:
            {
                rtl::Reference< unoidl::ExceptionTypeEntity > ent2(
                    static_cast< unoidl::ExceptionTypeEntity * >(
                        i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), !ent2->getDirectBase().isEmpty());
                if (!ent2->getDirectBase().isEmpty()) {
                    writeNameLen(file, ent2->getDirectBase());
                }
                write32(file, ent2->getDirectMembers().size());
                for (std::vector< unoidl::ExceptionTypeEntity::Member >::
                         const_iterator j(ent2->getDirectMembers().begin());
                     j != ent2->getDirectMembers().end(); ++j)
                {
                    writeNameLen(file, j->name);
                    writeNameLen(file, j->type);
                }
                break;
            }
        case unoidl::Entity::SORT_INTERFACE_TYPE:
            {
                rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
                    static_cast< unoidl::InterfaceTypeEntity * >(
                        i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get());
                write32(file, ent2->getDirectMandatoryBases().size());
                for (std::vector< OUString >::const_iterator j(
                         ent2->getDirectMandatoryBases().begin());
                     j != ent2->getDirectMandatoryBases().end(); ++j)
                {
                    writeNameLen(file, *j);
                }
                write32(file, ent2->getDirectOptionalBases().size());
                for (std::vector< OUString >::const_iterator j(
                         ent2->getDirectOptionalBases().begin());
                     j != ent2->getDirectOptionalBases().end(); ++j)
                {
                    writeNameLen(file, *j);
                }
                write32(file, ent2->getDirectAttributes().size());
                for (std::vector< unoidl::InterfaceTypeEntity::Attribute >::
                         const_iterator j(ent2->getDirectAttributes().begin());
                     j != ent2->getDirectAttributes().end(); ++j)
                {
                    sal_uInt64 f = 0;
                    if (j->bound) {
                        f |= 0x01;
                    }
                    if (j->readOnly) {
                        f |= 0x02;
                    }
                    write8(file, f);
                    writeNameLen(file, j->name);
                    writeNameLen(file, j->type);
                    write32(file, j->getExceptions.size());
                    for (std::vector< OUString >::const_iterator k(
                             j->getExceptions.begin());
                         k != j->getExceptions.end(); ++k)
                    {
                        writeNameLen(file, *k);
                    }
                    if (!j->readOnly) {
                        write32(file, j->setExceptions.size());
                        for (std::vector< OUString >::const_iterator k(
                                 j->setExceptions.begin());
                             k != j->setExceptions.end(); ++k)
                        {
                            writeNameLen(file, *k);
                        }
                    }
                }
                write32(file, ent2->getDirectMethods().size());
                for (std::vector< unoidl::InterfaceTypeEntity::Method >::
                         const_iterator j(ent2->getDirectMethods().begin());
                     j != ent2->getDirectMethods().end(); ++j)
                {
                    writeNameLen(file, j->name);
                    writeNameLen(file, j->returnType);
                    write32(file, j->parameters.size());
                    for (std::vector<
                             unoidl::InterfaceTypeEntity::Method::Parameter >::
                             const_iterator k(j->parameters.begin());
                         k != j->parameters.end(); ++k)
                    {
                        write8(file, k->direction);
                        writeNameLen(file, k->name);
                        writeNameLen(file, k->type);
                    }
                    write32(file, j->exceptions.size());
                    for (std::vector< OUString >::const_iterator k(
                             j->exceptions.begin());
                         k != j->exceptions.end(); ++k)
                    {
                        writeNameLen(file, *k);
                    }
                }
                break;
            }
        case unoidl::Entity::SORT_TYPEDEF:
            {
                rtl::Reference< unoidl::TypedefEntity > ent2(
                    static_cast< unoidl::TypedefEntity * >(
                        i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get());
                writeNameLen(file, ent2->getType());
                break;
            }
        case unoidl::Entity::SORT_CONSTANT_GROUP:
            {
                rtl::Reference< unoidl::ConstantGroupEntity > ent2(
                    static_cast< unoidl::ConstantGroupEntity * >(
                        i->second.entity.get()));
                std::map< OUString, ConstItem > cmap;
                for (std::vector< unoidl::ConstantGroupEntity::Member >::
                         const_iterator j(ent2->getMembers().begin());
                     j != ent2->getMembers().end(); ++j)
                {
                    if (!cmap.insert(
                            std::make_pair(j->name, ConstItem(j->value))).
                        second)
                    {
                        std::cout
                            << "Duplicate constant group member name \""
                            << j->name << '"' << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                }
                for (std::map< OUString, ConstItem >::iterator j(cmap.begin());
                     j != cmap.end(); ++j)
                {
                    j->second.dataOffset = getOffset(file);
                    sal_uInt64 v = j->second.constant.type;
                    if (false /*TODO: deprecated */) {
                        v |= 0x80;
                    }
                    write8(file, v);
                    switch (j->second.constant.type) {
                    case unoidl::ConstantValue::TYPE_BOOLEAN:
                        write8(file, j->second.constant.booleanValue ? 1 : 0);
                        break;
                    case unoidl::ConstantValue::TYPE_BYTE:
                        write8(
                            file,
                            static_cast< sal_uInt8 >(
                                j->second.constant.byteValue));
                        break;
                    case unoidl::ConstantValue::TYPE_SHORT:
                        write16(
                            file,
                            static_cast< sal_uInt16 >(
                                j->second.constant.shortValue));
                        break;
                    case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
                        write16(file, j->second.constant.unsignedShortValue);
                        break;
                    case unoidl::ConstantValue::TYPE_LONG:
                        write32(
                            file,
                            static_cast< sal_uInt32 >(
                                j->second.constant.longValue));
                        break;
                    case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
                        write32(file, j->second.constant.unsignedLongValue);
                        break;
                    case unoidl::ConstantValue::TYPE_HYPER:
                        write64(
                            file,
                            static_cast< sal_uInt64 >(
                                j->second.constant.hyperValue));
                        break;
                    case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
                        write64(file, j->second.constant.unsignedHyperValue);
                        break;
                    case unoidl::ConstantValue::TYPE_FLOAT:
                        writeIso60599Binary32(
                            file, j->second.constant.floatValue);
                        break;
                    case unoidl::ConstantValue::TYPE_DOUBLE:
                        writeIso60599Binary64(
                            file, j->second.constant.doubleValue);
                        break;
                    default:
                        for (;;) { std::abort(); } // this cannot happen
                    }
                }
                for (std::map< OUString, ConstItem >::iterator j(
                         cmap.begin());
                     j != cmap.end(); ++j)
                {
                    j->second.nameOffset = writeNameNul(file, j->first);
                }
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get());
                write32(file, cmap.size());
                    // overflow from std::map::size_type -> sal_uInt64 is
                    // unrealistic
                for (std::map< OUString, ConstItem >::iterator j(
                         cmap.begin());
                     j != cmap.end(); ++j)
                {
                    write32(file, j->second.nameOffset);
                    write32(file, j->second.dataOffset);
                }
                break;
            }
        case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
            {
                rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity >
                    ent2(
                        static_cast<
                            unoidl::SingleInterfaceBasedServiceEntity * >(
                                i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                bool dfltCtor = ent2->getConstructors().size() == 1
                    && ent2->getConstructors()[0].defaultConstructor;
                writeKind(file, ent2.get(), dfltCtor);
                writeNameLen(file, ent2->getBase());
                if (!dfltCtor) {
                    write32(file, ent2->getConstructors().size());
                    for (std::vector<
                             unoidl::SingleInterfaceBasedServiceEntity::
                             Constructor >::const_iterator j(
                                 ent2->getConstructors().begin());
                         j != ent2->getConstructors().end(); ++j)
                    {
                        if (j->defaultConstructor) {
                            std::cout
                                << "Unexpected default constructor \""
                                << j->name << '"' << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                        writeNameLen(file, j->name);
                        write32(file, j->parameters.size());
                        for (std::vector<
                                 unoidl::SingleInterfaceBasedServiceEntity::
                                 Constructor::Parameter >::const_iterator k(
                                     j->parameters.begin());
                             k != j->parameters.end(); ++k)
                        {
                            sal_uInt64 f = 0;
                            if (k->rest) {
                                f |= 0x04;
                            }
                            write8(file, f);
                            writeNameLen(file, k->name);
                            writeNameLen(file, k->type);
                        }
                        write32(file, j->exceptions.size());
                        for (std::vector< OUString >::const_iterator k(
                                 j->exceptions.begin());
                             k != j->exceptions.end(); ++k)
                        {
                            writeNameLen(file, *k);
                        }
                    }
                }
                break;
            }
        case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
            {
                rtl::Reference< unoidl::AccumulationBasedServiceEntity > ent2(
                    static_cast< unoidl::AccumulationBasedServiceEntity * >(
                        i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get());
                write32(file, ent2->getDirectMandatoryBaseServices().size());
                for (std::vector< OUString >::const_iterator j(
                         ent2->getDirectMandatoryBaseServices().begin());
                     j != ent2->getDirectMandatoryBaseServices().end(); ++j)
                {
                    writeNameLen(file, *j);
                }
                write32(file, ent2->getDirectOptionalBaseServices().size());
                for (std::vector< OUString >::const_iterator j(
                         ent2->getDirectOptionalBaseServices().begin());
                     j != ent2->getDirectOptionalBaseServices().end(); ++j)
                {
                    writeNameLen(file, *j);
                }
                write32(file, ent2->getDirectMandatoryBaseInterfaces().size());
                for (std::vector< OUString >::const_iterator j(
                         ent2->getDirectMandatoryBaseInterfaces().begin());
                     j != ent2->getDirectMandatoryBaseInterfaces().end(); ++j)
                {
                    writeNameLen(file, *j);
                }
                write32(file, ent2->getDirectOptionalBaseInterfaces().size());
                for (std::vector< OUString >::const_iterator j(
                         ent2->getDirectOptionalBaseInterfaces().begin());
                     j != ent2->getDirectOptionalBaseInterfaces().end(); ++j)
                {
                    writeNameLen(file, *j);
                }
                write32(file, ent2->getDirectProperties().size());
                for (std::vector<
                         unoidl::AccumulationBasedServiceEntity::Property >::
                         const_iterator j(
                             ent2->getDirectProperties().begin());
                     j != ent2->getDirectProperties().end(); ++j)
                {
                    write16(file, static_cast< sal_uInt16 >(j->attributes));
                    writeNameLen(file, j->name);
                    writeNameLen(file, j->type);
                }
                break;
            }
        case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
            {
                rtl::Reference< unoidl::InterfaceBasedSingletonEntity > ent2(
                    static_cast< unoidl::InterfaceBasedSingletonEntity * >(
                        i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get());
                writeNameLen(file, ent2->getBase());
                break;
            }
        case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
            {
                rtl::Reference< unoidl::ServiceBasedSingletonEntity > ent2(
                    static_cast< unoidl::ServiceBasedSingletonEntity * >(
                        i->second.entity.get()));
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get());
                writeNameLen(file, ent2->getBase());
                break;
            }
        }
    }
    for (std::map< OUString, Item >::iterator i(map.begin()); i != map.end();
         ++i)
    {
        i->second.nameOffset = writeNameNul(file, i->first);
    }
    sal_uInt64 off = getOffset(file);
    if (rootSize == 0) {
        write8(file, 0); // SORT_MODULE
        write32(file, map.size());
            // overflow from std::map::size_type -> sal_uInt64 is unrealistic
    } else {
        *rootSize = map.size();
            // overflow from std::map::size_type -> std::size_t is unrealistic
    }
    for (std::map< OUString, Item >::iterator i(map.begin()); i != map.end();
         ++i)
    {
        write32(file, i->second.nameOffset);
        write32(file, i->second.dataOffset);
    }
    return off;
}

}

SAL_IMPLEMENT_MAIN() {
    sal_uInt32 args = rtl_getAppCommandArgCount();
    if (args < 2) {
        std::cerr
            << "Usage: reg2unoidl <extra .rdb files> <.rdb file> <unoidl file>"
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
    rtl::Reference< unoidl::Manager > mgr(new unoidl::Manager);
    for (sal_uInt32 i = 0; i != args - 2; ++i) {
        mgr->addProvider(load(mgr, getArgumentUri(i)));
    }
    rtl::Reference< unoidl::Provider > prov(
        load(mgr, getArgumentUri(args - 2)));
    osl::File f(getArgumentUri(args - 1));
    osl::FileBase::RC e = f.open(osl_File_OpenFlag_Write);
    if (e == osl::FileBase::E_NOENT) {
        e = f.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create);
    }
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot open <" << f.getURL() << "> for writing, error code "
            << +e << std::endl;
        std::exit(EXIT_FAILURE);
    }
    write(f, "UNOIDL\0\xFF", 8);
    write32(f, 0); // root map offset
    write32(f, 0); // root map size
    sal_uInt64 off;
    std::size_t size;
    try {
        off = writeMap(f, prov->createRootCursor(), &size);
    } catch (unoidl::FileFormatException & e) {
        std::cerr
            << "Bad input <" << e.getUri() << ">: " << e.getDetail()
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
    e = f.setSize(getOffset(f)); // truncate in case it already existed
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot set size of <" << f.getURL() << ">, error code " << +e
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
    e = f.setPos(osl_Pos_Absolut, 8);
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot rewind current position in <" << f.getURL()
            << ">, error code " << +e << std::endl;
        std::exit(EXIT_FAILURE);
    }
    write32(f, off);
    write32(f, size);
        // overflow from std::map::size_type -> sal_uInt64 is unrealistic
    e = f.close();
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot close <" << f.getURL() << "> after writing, error code "
            << +e << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
