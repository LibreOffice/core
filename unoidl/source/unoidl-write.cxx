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

#include "config_version.h"
#include "osl/endian.h"
#include "osl/file.h"
#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/process.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/textcvt.h"
#include "rtl/ustring.hxx"
#include "sal/macros.h"
#include "sal/main.h"
#include "unoidl/unoidl.hxx"

namespace {

void badUsage() {
    std::cerr
        << "Usage:" << std::endl << std::endl
        << "  unoidl-write [<registries>] [@<entities file>] <unoidl file>"
        << std::endl << std::endl
        << ("where each <registry> is either a new- or legacy-format .rdb"
            " file or a")
        << std::endl
        << ("root directory of an .idl file tree, and the UTF-8 encoded"
            " <entities file>")
        << std::endl
        << ("contains zero or more space-separated names of (non-module)"
            " entities to include")
        << std::endl
        << ("in the output, and, if omitted, defaults to the complete content"
            " of the final")
        << std::endl << "<registry>, if any." << std::endl;
    std::exit(EXIT_FAILURE);
}

OUString getArgumentUri(sal_uInt32 argument, bool * entities) {
    OUString arg;
    rtl_getAppCommandArg(argument, &arg.pData);
    if (arg.startsWith("@")) {
        if (entities == 0) {
            badUsage();
        }
        *entities = true;
        arg = arg.copy(1);
    } else if (entities != 0) {
        *entities = false;
    }
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
    buf[4] = (value >> 32) & 0xFF;
    buf[5] = (value >> 40) & 0xFF;
    buf[6] = (value >> 48) & 0xFF;
    buf[7] = (value >> 56) & 0xFF;
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

OString toUtf8(OUString const & string) {
    OString ascii;
    if (!string.convertToString(
            &ascii, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        std::cerr
            << "Cannot convert \"" << string << "\" to UTF-8" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return ascii;
}

sal_uInt64 writeNulName(osl::File & file, OUString const & name) {
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

void writeIdxString(osl::File & file, OString const & string) {
    static std::map< OString, sal_uInt64 > reuse;
    std::map< OString, sal_uInt64 >::iterator i(reuse.find(string));
    if (i == reuse.end()) {
        reuse.insert(std::make_pair(string, getOffset(file)));
        assert(
            (static_cast< sal_uInt64 >(string.getLength()) & 0x80000000) == 0);
        write32(file, static_cast< sal_uInt64 >(string.getLength()));
        write(file, string.getStr(), string.getLength());
    } else {
        if ((i->second & 0x80000000) != 0) {
            std::cerr
                << "Cannot write index 0x" << std::hex << i->second << std::dec
                << " of \"" << string << "\"; input is too large" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        write32(file, i->second | 0x80000000);
    }
}

void writeIdxName(osl::File & file, OUString const & name) {
    writeIdxString(file, toAscii(name));
}

void writeAnnotations(
    osl::File & file, bool annotate,
    std::vector< OUString > const & annotations)
{
    assert(annotate || annotations.empty());
    if (annotate) {
        write32(file, annotations.size());
            // overflow from std::vector::size_type -> sal_uInt64 is unrealistic
        for (std::vector< OUString >::const_iterator i(annotations.begin());
             i != annotations.end(); ++i)
        {
            writeIdxString(file, toUtf8(*i));
        }
    }
}

void writeKind(
    osl::File & file,
    rtl::Reference< unoidl::PublishableEntity > const & entity,
    bool annotated, bool flag = false)
{
    assert(entity.is());
    sal_uInt64 v = entity->getSort();
    if (entity->isPublished()) {
        v |= 0x80;
    }
    if (annotated) {
        v |= 0x40;
    }
    if (flag) {
        v |= 0x20;
    }
    write8(file, v);
}

struct Item {
    explicit Item(rtl::Reference< unoidl::Entity > const & theEntity):
        entity(theEntity), nameOffset(0), dataOffset(0)
    {}

    rtl::Reference< unoidl::Entity > entity;
    std::map< OUString, Item > module;
    sal_uInt64 nameOffset;
    sal_uInt64 dataOffset;
};

struct ConstItem {
    ConstItem(
        unoidl::ConstantValue const & theConstant,
        std::vector< OUString > const & theAnnotations):
        constant(theConstant), annotations(theAnnotations), nameOffset(0),
        dataOffset(0)
    {}

    unoidl::ConstantValue constant;
    std::vector< OUString > annotations;
    sal_uInt64 nameOffset;
    sal_uInt64 dataOffset;
};

void mapEntities(
    rtl::Reference< unoidl::Manager > const & manager, OUString const & uri,
    std::map< OUString, Item > & map)
{
    assert(manager.is());
    osl::File f(uri);
    osl::FileBase::RC e = f.open(osl_File_OpenFlag_Read);
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot open <" << f.getURL() << "> for reading, error code "
            << +e << std::endl;
        std::exit(EXIT_FAILURE);
    }
    for (;;) {
        sal_Bool eof;
        e = f.isEndOfFile(&eof);
        if (e != osl::FileBase::E_None) {
            std::cerr
                << "Cannot check <" << f.getURL() << "> for EOF, error code "
                << +e << std::endl;
            std::exit(EXIT_FAILURE);
        }
        if (eof) {
            break;
        }
        rtl::ByteSequence s1;
        e = f.readLine(s1);
        if (e != osl::FileBase::E_None) {
            std::cerr
                << "Cannot read from <" << f.getURL() << ">, error code "
                << +e << std::endl;
            std::exit(EXIT_FAILURE);
        }
        OUString s2;
        if (!rtl_convertStringToUString(
                &s2.pData, reinterpret_cast< char const * >(s1.getConstArray()),
                s1.getLength(), RTL_TEXTENCODING_UTF8,
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
        {
            std::cerr
                << "Cannot interpret line read from <" << f.getURL()
                << "> as UTF-8" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        for (sal_Int32 i = 0; i != -1;) {
            OUString t(s2.getToken(0, ' ', i));
            if (!t.isEmpty()) {
                rtl::Reference< unoidl::Entity > ent(manager->findEntity(t));
                if (!ent.is()) {
                    std::cerr
                        << "Unknown entity \"" << t << "\" read from <"
                        << f.getURL() << ">" << std::endl;
                    std::exit(EXIT_FAILURE);
                }
                if (ent->getSort() == unoidl::Entity::SORT_MODULE) {
                    std::cerr
                        << "Module entity \"" << t << "\" read from <"
                        << f.getURL() << ">" << std::endl;
                    std::exit(EXIT_FAILURE);
                }
                std::map< OUString, Item > * map2 = &map;
                for (sal_Int32 j = 0;;) {
                    OUString id(t.getToken(0, '.', j));
                    if (j == -1) {
                        map2->insert(std::make_pair(id, Item(ent)));
                        break;
                    }
                    std::map< OUString, Item >::iterator k(map2->find(id));
                    if (k == map2->end()) {
                        rtl::Reference< unoidl::Entity > ent2(
                            manager->findEntity(t.copy(0, j - 1)));
                        assert(ent2.is());
                        k = map2->insert(std::make_pair(id, Item(ent2))).first;
                    }
                    assert(
                        k->second.entity->getSort()
                        == unoidl::Entity::SORT_MODULE);
                    map2 = &k->second.module;
                }
            }
        }
    }
    e = f.close();
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot close <" << f.getURL() << "> after reading, error code "
            << +e << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void mapCursor(
    rtl::Reference< unoidl::MapCursor > const & cursor,
    std::map< OUString, Item > & map)
{
    if (cursor.is()) {
        for (;;) {
            OUString name;
            rtl::Reference< unoidl::Entity > ent(cursor->getNext(&name));
            if (!ent.is()) {
                break;
            }
            std::pair< std::map< OUString, Item >::iterator, bool > i(
                map.insert(std::make_pair(name, Item(ent))));
            if (!i.second) {
                std::cout << "Duplicate name \"" << name << '"' << std::endl;
                std::exit(EXIT_FAILURE);
            }
            if (i.first->second.entity->getSort()
                == unoidl::Entity::SORT_MODULE)
            {
                mapCursor(
                    rtl::Reference< unoidl::ModuleEntity >(
                        static_cast< unoidl::ModuleEntity * >(
                            i.first->second.entity.get()))->createCursor(),
                    i.first->second.module);
            }
        }
    }
}

sal_uInt64 writeMap(
    osl::File & file, std::map< OUString, Item > & map, std::size_t * rootSize)
{
    for (std::map< OUString, Item >::iterator i(map.begin()); i != map.end();
         ++i)
    {
        switch (i->second.entity->getSort()) {
        case unoidl::Entity::SORT_MODULE:
            i->second.dataOffset = writeMap(file, i->second.module, 0);
            break;
        case unoidl::Entity::SORT_ENUM_TYPE:
            {
                rtl::Reference< unoidl::EnumTypeEntity > ent2(
                    static_cast< unoidl::EnumTypeEntity * >(
                        i->second.entity.get()));
                bool ann = !ent2->getAnnotations().empty();
                for (std::vector< unoidl::EnumTypeEntity::Member >::
                         const_iterator j(ent2->getMembers().begin());
                     !ann && j != ent2->getMembers().end(); ++j)
                {
                    ann = !j->annotations.empty();
                }
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), ann);
                write32(file, ent2->getMembers().size());
                for (std::vector< unoidl::EnumTypeEntity::Member >::
                         const_iterator j(ent2->getMembers().begin());
                     j != ent2->getMembers().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    write32(file, static_cast< sal_uInt32 >(j->value));
                    writeAnnotations(file, ann, j->annotations);
                }
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
            {
                rtl::Reference< unoidl::PlainStructTypeEntity > ent2(
                    static_cast< unoidl::PlainStructTypeEntity * >(
                        i->second.entity.get()));
                bool ann = !ent2->getAnnotations().empty();
                for (std::vector< unoidl::PlainStructTypeEntity::Member >::
                         const_iterator j(ent2->getDirectMembers().begin());
                     !ann && j != ent2->getDirectMembers().end(); ++j)
                {
                    ann = !j->annotations.empty();
                }
                i->second.dataOffset = getOffset(file);
                writeKind(
                    file, ent2.get(), ann, !ent2->getDirectBase().isEmpty());
                if (!ent2->getDirectBase().isEmpty()) {
                    writeIdxName(file, ent2->getDirectBase());
                }
                write32(file, ent2->getDirectMembers().size());
                for (std::vector< unoidl::PlainStructTypeEntity::Member >::
                         const_iterator j(ent2->getDirectMembers().begin());
                     j != ent2->getDirectMembers().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    writeIdxName(file, j->type);
                    writeAnnotations(file, ann, j->annotations);
                }
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
            {
                rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity >
                    ent2(
                        static_cast<
                        unoidl::PolymorphicStructTypeTemplateEntity * >(
                            i->second.entity.get()));
                bool ann = !ent2->getAnnotations().empty();
                for (std::vector<
                         unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                         const_iterator j(
                             ent2->getMembers().begin());
                     !ann && j != ent2->getMembers().end(); ++j)
                {
                    ann = !j->annotations.empty();
                }
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), ann);
                write32(file, ent2->getTypeParameters().size());
                for (std::vector< OUString >::const_iterator j(
                         ent2->getTypeParameters().begin());
                     j != ent2->getTypeParameters().end(); ++j)
                {
                    writeIdxName(file, *j);
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
                    writeIdxName(file, j->name);
                    writeIdxName(file, j->type);
                    writeAnnotations(file, ann, j->annotations);
                }
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        case unoidl::Entity::SORT_EXCEPTION_TYPE:
            {
                rtl::Reference< unoidl::ExceptionTypeEntity > ent2(
                    static_cast< unoidl::ExceptionTypeEntity * >(
                        i->second.entity.get()));
                bool ann = !ent2->getAnnotations().empty();
                for (std::vector< unoidl::ExceptionTypeEntity::Member >::
                         const_iterator j(ent2->getDirectMembers().begin());
                     !ann && j != ent2->getDirectMembers().end(); ++j)
                {
                    ann = !j->annotations.empty();
                }
                i->second.dataOffset = getOffset(file);
                writeKind(
                    file, ent2.get(), ann, !ent2->getDirectBase().isEmpty());
                if (!ent2->getDirectBase().isEmpty()) {
                    writeIdxName(file, ent2->getDirectBase());
                }
                write32(file, ent2->getDirectMembers().size());
                for (std::vector< unoidl::ExceptionTypeEntity::Member >::
                         const_iterator j(ent2->getDirectMembers().begin());
                     j != ent2->getDirectMembers().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    writeIdxName(file, j->type);
                    writeAnnotations(file, ann, j->annotations);
                }
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        case unoidl::Entity::SORT_INTERFACE_TYPE:
            {
                rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
                    static_cast< unoidl::InterfaceTypeEntity * >(
                        i->second.entity.get()));
                bool ann = !ent2->getAnnotations().empty();
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectMandatoryBases().begin());
                     !ann && j != ent2->getDirectMandatoryBases().end(); ++j)
                {
                    ann = !j->annotations.empty();
                }
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectOptionalBases().begin());
                     !ann && j != ent2->getDirectOptionalBases().end(); ++j)
                {
                    ann = !j->annotations.empty();
                }
                for (std::vector< unoidl::InterfaceTypeEntity::Attribute >::
                         const_iterator j(ent2->getDirectAttributes().begin());
                     !ann && j != ent2->getDirectAttributes().end(); ++j)
                {
                    ann = !j->annotations.empty();
                }
                for (std::vector< unoidl::InterfaceTypeEntity::Method >::
                         const_iterator j(ent2->getDirectMethods().begin());
                     !ann && j != ent2->getDirectMethods().end(); ++j)
                {
                    ann = !j->annotations.empty();
                }
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), ann);
                write32(file, ent2->getDirectMandatoryBases().size());
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectMandatoryBases().begin());
                     j != ent2->getDirectMandatoryBases().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    writeAnnotations(file, ann, j->annotations);
                }
                write32(file, ent2->getDirectOptionalBases().size());
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectOptionalBases().begin());
                     j != ent2->getDirectOptionalBases().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    writeAnnotations(file, ann, j->annotations);
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
                    writeIdxName(file, j->name);
                    writeIdxName(file, j->type);
                    write32(file, j->getExceptions.size());
                    for (std::vector< OUString >::const_iterator k(
                             j->getExceptions.begin());
                         k != j->getExceptions.end(); ++k)
                    {
                        writeIdxName(file, *k);
                    }
                    if (!j->readOnly) {
                        write32(file, j->setExceptions.size());
                        for (std::vector< OUString >::const_iterator k(
                                 j->setExceptions.begin());
                             k != j->setExceptions.end(); ++k)
                        {
                            writeIdxName(file, *k);
                        }
                    }
                    writeAnnotations(file, ann, j->annotations);
                }
                write32(file, ent2->getDirectMethods().size());
                for (std::vector< unoidl::InterfaceTypeEntity::Method >::
                         const_iterator j(ent2->getDirectMethods().begin());
                     j != ent2->getDirectMethods().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    writeIdxName(file, j->returnType);
                    write32(file, j->parameters.size());
                    for (std::vector<
                             unoidl::InterfaceTypeEntity::Method::Parameter >::
                             const_iterator k(j->parameters.begin());
                         k != j->parameters.end(); ++k)
                    {
                        write8(file, k->direction);
                        writeIdxName(file, k->name);
                        writeIdxName(file, k->type);
                    }
                    write32(file, j->exceptions.size());
                    for (std::vector< OUString >::const_iterator k(
                             j->exceptions.begin());
                         k != j->exceptions.end(); ++k)
                    {
                        writeIdxName(file, *k);
                    }
                    writeAnnotations(file, ann, j->annotations);
                }
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        case unoidl::Entity::SORT_TYPEDEF:
            {
                rtl::Reference< unoidl::TypedefEntity > ent2(
                    static_cast< unoidl::TypedefEntity * >(
                        i->second.entity.get()));
                bool ann = !ent2->getAnnotations().empty();
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), ann);
                writeIdxName(file, ent2->getType());
                writeAnnotations(file, ann, ent2->getAnnotations());
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
                            std::make_pair(
                                j->name, ConstItem(j->value, j->annotations))).
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
                    if (!j->second.annotations.empty()) {
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
                    writeAnnotations(
                        file, !j->second.annotations.empty(),
                        j->second.annotations);
                }
                for (std::map< OUString, ConstItem >::iterator j(
                         cmap.begin());
                     j != cmap.end(); ++j)
                {
                    j->second.nameOffset = writeNulName(file, j->first);
                }
                bool ann = !ent2->getAnnotations().empty();
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), ann);
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
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
            {
                rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity >
                    ent2(
                        static_cast<
                            unoidl::SingleInterfaceBasedServiceEntity * >(
                                i->second.entity.get()));
                bool dfltCtor = ent2->getConstructors().size() == 1
                    && ent2->getConstructors()[0].defaultConstructor;
                bool ann = !ent2->getAnnotations().empty();
                if (!dfltCtor) {
                    for (std::vector<
                             unoidl::SingleInterfaceBasedServiceEntity::
                             Constructor >::const_iterator j(
                                 ent2->getConstructors().begin());
                         !ann && j != ent2->getConstructors().end(); ++j)
                    {
                        ann = !j->annotations.empty();
                    }
                }
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), ann, dfltCtor);
                writeIdxName(file, ent2->getBase());
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
                        writeIdxName(file, j->name);
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
                            writeIdxName(file, k->name);
                            writeIdxName(file, k->type);
                        }
                        write32(file, j->exceptions.size());
                        for (std::vector< OUString >::const_iterator k(
                                 j->exceptions.begin());
                             k != j->exceptions.end(); ++k)
                        {
                            writeIdxName(file, *k);
                        }
                        writeAnnotations(file, ann, j->annotations);
                    }
                }
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
            {
                rtl::Reference< unoidl::AccumulationBasedServiceEntity > ent2(
                    static_cast< unoidl::AccumulationBasedServiceEntity * >(
                        i->second.entity.get()));
                bool ann = !ent2->getAnnotations().empty();
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectMandatoryBaseServices().begin());
                     !ann && j != ent2->getDirectMandatoryBaseServices().end();
                     ++j)
                {
                    ann = !j->annotations.empty();
                }
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectOptionalBaseServices().begin());
                     !ann && j != ent2->getDirectOptionalBaseServices().end();
                     ++j)
                {
                    ann = !j->annotations.empty();
                }
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectMandatoryBaseInterfaces().begin());
                     (!ann
                      && j != ent2->getDirectMandatoryBaseInterfaces().end());
                     ++j)
                {
                    ann = !j->annotations.empty();
                }
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectOptionalBaseInterfaces().begin());
                     !ann && j != ent2->getDirectOptionalBaseInterfaces().end();
                     ++j)
                {
                    ann = !j->annotations.empty();
                }
                for (std::vector<
                         unoidl::AccumulationBasedServiceEntity::Property >::
                         const_iterator j(
                             ent2->getDirectProperties().begin());
                     !ann && j != ent2->getDirectProperties().end(); ++j)
                {
                    ann = !j->annotations.empty();
                }
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), ann);
                write32(file, ent2->getDirectMandatoryBaseServices().size());
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectMandatoryBaseServices().begin());
                     j != ent2->getDirectMandatoryBaseServices().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    writeAnnotations(file, ann, j->annotations);
                }
                write32(file, ent2->getDirectOptionalBaseServices().size());
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectOptionalBaseServices().begin());
                     j != ent2->getDirectOptionalBaseServices().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    writeAnnotations(file, ann, j->annotations);
                }
                write32(file, ent2->getDirectMandatoryBaseInterfaces().size());
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectMandatoryBaseInterfaces().begin());
                     j != ent2->getDirectMandatoryBaseInterfaces().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    writeAnnotations(file, ann, j->annotations);
                }
                write32(file, ent2->getDirectOptionalBaseInterfaces().size());
                for (std::vector< unoidl::AnnotatedReference >::const_iterator
                         j(ent2->getDirectOptionalBaseInterfaces().begin());
                     j != ent2->getDirectOptionalBaseInterfaces().end(); ++j)
                {
                    writeIdxName(file, j->name);
                    writeAnnotations(file, ann, j->annotations);
                }
                write32(file, ent2->getDirectProperties().size());
                for (std::vector<
                         unoidl::AccumulationBasedServiceEntity::Property >::
                         const_iterator j(
                             ent2->getDirectProperties().begin());
                     j != ent2->getDirectProperties().end(); ++j)
                {
                    write16(file, static_cast< sal_uInt16 >(j->attributes));
                    writeIdxName(file, j->name);
                    writeIdxName(file, j->type);
                    writeAnnotations(file, ann, j->annotations);
                }
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
            {
                rtl::Reference< unoidl::InterfaceBasedSingletonEntity > ent2(
                    static_cast< unoidl::InterfaceBasedSingletonEntity * >(
                        i->second.entity.get()));
                bool ann = !ent2->getAnnotations().empty();
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), ann);
                writeIdxName(file, ent2->getBase());
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
            {
                rtl::Reference< unoidl::ServiceBasedSingletonEntity > ent2(
                    static_cast< unoidl::ServiceBasedSingletonEntity * >(
                        i->second.entity.get()));
                bool ann = !ent2->getAnnotations().empty();
                i->second.dataOffset = getOffset(file);
                writeKind(file, ent2.get(), ann);
                writeIdxName(file, ent2->getBase());
                writeAnnotations(file, ann, ent2->getAnnotations());
                break;
            }
        }
    }
    for (std::map< OUString, Item >::iterator i(map.begin()); i != map.end();
         ++i)
    {
        i->second.nameOffset = writeNulName(file, i->first);
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
    try {
        sal_uInt32 args = rtl_getAppCommandArgCount();
        if (args == 0) {
            badUsage();
        }
        rtl::Reference< unoidl::Manager > mgr(new unoidl::Manager);
        bool entities = false;
        rtl::Reference< unoidl::Provider > prov;
        std::map< OUString, Item > map;
        for (sal_uInt32 i = 0; i != args - 1; ++i) {
            assert(args > 1);
            OUString uri(getArgumentUri(i, i == args - 2 ? &entities : 0));
            if (entities) {
                mapEntities(mgr, uri, map);
            } else {
                try {
                    prov = unoidl::loadProvider(mgr, uri);
                } catch (unoidl::NoSuchFileException &) {
                    std::cerr
                        << "Input <" << uri << "> does not exist" << std::endl;
                    std::exit(EXIT_FAILURE);
                }
                mgr->addProvider(prov);
            }
        }
        if (!entities) {
            mapCursor(
                (prov.is()
                 ? prov->createRootCursor()
                 : rtl::Reference< unoidl::MapCursor >()),
                map);
        }
        osl::File f(getArgumentUri(args - 1, 0));
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
        write(f, "UNOIDL\xFF\0", 8);
        write32(f, 0); // root map offset
        write32(f, 0); // root map size
        write(
            f,
            RTL_CONSTASCII_STRINGPARAM(
                "\0** Created by LibreOffice " LIBO_VERSION_DOTTED
                " unoidl-write **\0"));
        std::size_t size;
        sal_uInt64 off = writeMap(f, map, &size);
        e = f.setSize(getOffset(f)); // truncate in case it already existed
        if (e != osl::FileBase::E_None) {
            std::cerr
                << "Cannot set size of <" << f.getURL() << ">, error code "
                << +e << std::endl;
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
                << "Cannot close <" << f.getURL()
                << "> after writing, error code " << +e << std::endl;
            std::exit(EXIT_FAILURE);
        }
        return EXIT_SUCCESS;
    } catch (unoidl::FileFormatException & e1) {
        std::cerr
            << "Bad input <" << e1.getUri() << ">: " << e1.getDetail()
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
