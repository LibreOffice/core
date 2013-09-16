/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

#include "osl/endian.h"
#include "osl/file.h"
#include "rtl/ref.hxx"
#include "rtl/textenc.h"
#include "rtl/textcvt.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"
#include "unoidl/unoidl.hxx"

#include "unoidlprovider.hxx"

namespace unoidl { namespace detail {

namespace {

// sizeof (Memory16) == 2
struct Memory16 {
    unsigned char byte[2];

    sal_uInt16 getUnsigned16() const {
        return static_cast< sal_uInt16 >(byte[0])
            | (static_cast< sal_uInt16 >(byte[1]) << 8);
    }
};

// sizeof (Memory32) == 4
struct Memory32 {
    unsigned char byte[4];

    sal_uInt32 getUnsigned32() const {
        return static_cast< sal_uInt32 >(byte[0])
            | (static_cast< sal_uInt32 >(byte[1]) << 8)
            | (static_cast< sal_uInt32 >(byte[2]) << 16)
            | (static_cast< sal_uInt32 >(byte[3]) << 24);
    }

    float getIso60599Binary32() const {
        union {
            unsigned char buf[4];
            float f; // assuming float is ISO 60599 binary32
        } sa;
#if defined OSL_LITENDIAN
        sa.buf[0] = byte[0];
        sa.buf[1] = byte[1];
        sa.buf[2] = byte[2];
        sa.buf[3] = byte[3];
#else
        sa.buf[0] = byte[3];
        sa.buf[1] = byte[2];
        sa.buf[2] = byte[1];
        sa.buf[3] = byte[0];
#endif
        return sa.f;
    }
};

// sizeof (Memory64) == 8
struct Memory64 {
    unsigned char byte[8];

    sal_uInt64 getUnsigned64() const {
        return static_cast< sal_uInt64 >(byte[0])
            | (static_cast< sal_uInt64 >(byte[1]) << 8)
            | (static_cast< sal_uInt64 >(byte[2]) << 16)
            | (static_cast< sal_uInt64 >(byte[3]) << 24)
            | (static_cast< sal_uInt64 >(byte[4]) << 32)
            | (static_cast< sal_uInt64 >(byte[5]) << 40)
            | (static_cast< sal_uInt64 >(byte[6]) << 48)
            | (static_cast< sal_uInt64 >(byte[7]) << 56);
        }

    double getIso60599Binary64() const {
        union {
            unsigned char buf[8];
            double d; // assuming double is ISO 60599 binary64
        } sa;
#if defined OSL_LITENDIAN
        sa.buf[0] = byte[0];
        sa.buf[1] = byte[1];
        sa.buf[2] = byte[2];
        sa.buf[3] = byte[3];
        sa.buf[4] = byte[4];
        sa.buf[5] = byte[5];
        sa.buf[6] = byte[6];
        sa.buf[7] = byte[7];
#else
        sa.buf[0] = byte[7];
        sa.buf[1] = byte[6];
        sa.buf[2] = byte[5];
        sa.buf[3] = byte[4];
        sa.buf[4] = byte[3];
        sa.buf[5] = byte[2];
        sa.buf[6] = byte[1];
        sa.buf[7] = byte[0];
#endif
        return sa.d;
    }
};

}

class MappedFile: public salhelper::SimpleReferenceObject {
public:
    explicit MappedFile(OUString const & fileUrl);

    sal_uInt8 read8(sal_uInt32 offset) const;

    sal_uInt16 read16(sal_uInt32 offset) const;

    sal_uInt32 read32(sal_uInt32 offset) const;

    sal_uInt64 read64(sal_uInt32 offset) const;

    float readIso60599Binary32(sal_uInt32 offset) const;

    double readIso60599Binary64(sal_uInt32 offset) const;

    OUString readNulName(sal_uInt32 offset) const;

    OUString readIdxName(sal_uInt32 * offset) const
    { return readIdxString(offset, RTL_TEXTENCODING_ASCII_US); }

    OUString readIdxString(sal_uInt32 * offset) const
    { return readIdxString(offset, RTL_TEXTENCODING_UTF8); }

    OUString uri;
    oslFileHandle handle;
    sal_uInt64 size;
    void * address;

private:
    virtual ~MappedFile();

    sal_uInt8 get8(sal_uInt32 offset) const;

    sal_uInt16 get16(sal_uInt32 offset) const;

    sal_uInt32 get32(sal_uInt32 offset) const;

    sal_uInt64 get64(sal_uInt32 offset) const;

    float getIso60599Binary32(sal_uInt32 offset) const;

    double getIso60599Binary64(sal_uInt32 offset) const;

    OUString readIdxString(sal_uInt32 * offset, rtl_TextEncoding encoding)
        const;
};

MappedFile::MappedFile(OUString const & fileUrl): uri(fileUrl) {
    oslFileError e = osl_openFile(uri.pData, &handle, osl_File_OpenFlag_Read);
    switch (e) {
    case osl_File_E_None:
        break;
    case osl_File_E_NOENT:
        throw NoSuchFileException(uri);
    default:
        throw FileFormatException(uri, "cannot open: " + OUString::number(e));
    }
    e = osl_getFileSize(handle, &size);
    if (e == osl_File_E_None) {
        e = osl_mapFile(
            handle, &address, size, 0, osl_File_MapFlag_RandomAccess);
    }
    if (e != osl_File_E_None) {
        oslFileError e2 = osl_closeFile(handle);
        SAL_WARN_IF(
            e2 != osl_File_E_None, "unoidl",
            "cannot close " << uri << ": " << +e2);
        throw FileFormatException(uri, "cannot mmap: " + OUString::number(e));
    }
}

sal_uInt8 MappedFile::read8(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 1) {
        throw FileFormatException(
            uri, "UNOIDL format: offset for 8-bit value too large");
    }
    return get8(offset);
}

sal_uInt16 MappedFile::read16(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 2) {
        throw FileFormatException(
            uri, "UNOIDL format: offset for 16-bit value too large");
    }
    return get16(offset);
}

sal_uInt32 MappedFile::read32(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 4) {
        throw FileFormatException(
            uri, "UNOIDL format: offset for 32-bit value too large");
    }
    return get32(offset);
}

sal_uInt64 MappedFile::read64(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 8) {
        throw FileFormatException(
            uri, "UNOIDL format: offset for 64-bit value too large");
    }
    return get64(offset);
}

float MappedFile::readIso60599Binary32(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 4) {
        throw FileFormatException(
            uri, "UNOIDL format: offset for 32-bit value too large");
    }
    return getIso60599Binary32(offset);
}

double MappedFile::readIso60599Binary64(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 8) {
        throw FileFormatException(
            uri, "UNOIDL format: offset for 64-bit value too large");
    }
    return getIso60599Binary64(offset);
}

OUString MappedFile::readNulName(sal_uInt32 offset) const {
    if (offset > size) {
        throw FileFormatException(
            uri, "UNOIDL format: offset for string too large");
    }
    sal_uInt64 end = offset;
    for (;; ++end) {
        if (end == size) {
            throw FileFormatException(
                uri, "UNOIDL format: string misses trailing NUL");
        }
        if (static_cast< char const * >(address)[end] == 0) {
            break;
        }
    }
    if (end - offset > SAL_MAX_INT32) {
        throw FileFormatException(uri, "UNOIDL format: string too long");
    }
    OUString name;
    if (!rtl_convertStringToUString(
            &name.pData, static_cast< char const * >(address) + offset,
            end - offset, RTL_TEXTENCODING_ASCII_US,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        throw FileFormatException(uri, "UNOIDL format: name is not ASCII");
    }
    return name;
}

MappedFile::~MappedFile() {
    oslFileError e = osl_unmapMappedFile(handle, address, size);
    SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot unmap: " << +e);
    e = osl_closeFile(handle);
    SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot close: " << +e);
}

sal_uInt8 MappedFile::get8(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 1);
    return static_cast< char const * >(address)[offset];
}

sal_uInt16 MappedFile::get16(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 2);
    return reinterpret_cast< Memory16 const * >(
        static_cast< char const * >(address) + offset)->getUnsigned16();
}

sal_uInt32 MappedFile::get32(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 4);
    return reinterpret_cast< Memory32 const * >(
        static_cast< char const * >(address) + offset)->getUnsigned32();
}

sal_uInt64 MappedFile::get64(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 8);
    return reinterpret_cast< Memory64 const * >(
        static_cast< char const * >(address) + offset)->getUnsigned64();
}

float MappedFile::getIso60599Binary32(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 4);
    return reinterpret_cast< Memory32 const * >(
        static_cast< char const * >(address) + offset)->getIso60599Binary32();
}

double MappedFile::getIso60599Binary64(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 8);
    return reinterpret_cast< Memory64 const * >(
        static_cast< char const * >(address) + offset)->getIso60599Binary64();
}

OUString MappedFile::readIdxString(
    sal_uInt32 * offset, rtl_TextEncoding encoding) const
{
    assert(offset != 0);
    sal_uInt32 len = read32(*offset);
    sal_uInt32 off;
    if ((len & 0x80000000) == 0) {
        off = *offset;
        *offset += 4 + len;
    } else {
        *offset += 4;
        off = len & ~0x80000000;
        len = read32(off);
        if ((len & 0x80000000) != 0) {
            throw FileFormatException(
                uri, "UNOIDL format: string length high bit set");
        }
    }
    if (len > SAL_MAX_INT32 || len > size - off - 4) {
        throw FileFormatException(
            uri, "UNOIDL format: size of string is too large");
    }
    OUString name;
    if (!rtl_convertStringToUString(
            &name.pData, static_cast< char const * >(address) + off + 4, len,
            encoding,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        throw FileFormatException(
            uri, "UNOIDL format: string bytes do not match encoding");
    }
    return name;
}

// sizeof (MapEntry) == 8
struct MapEntry {
    Memory32 name;
    Memory32 data;
};

namespace {

enum Compare { COMPARE_LESS, COMPARE_GREATER, COMPARE_EQUAL };

Compare compare(
    rtl::Reference< MappedFile > const & file, OUString const & name,
    sal_Int32 nameOffset, sal_Int32 nameLength, MapEntry const * entry)
{
    assert(file.is());
    assert(entry != 0);
    sal_uInt32 off = entry->name.getUnsigned32();
    if (off > file->size - 1) { // at least a trailing NUL
        throw FileFormatException(
            file->uri, "UNOIDL format: string offset too large");
    }
    assert(nameLength >= 0);
    sal_uInt64 min = std::min(
        static_cast< sal_uInt64 >(nameLength), file->size - off);
    for (sal_uInt64 i = 0; i != min; ++i) {
        sal_Unicode c1 = name[nameOffset + i];
        sal_Unicode c2 = static_cast< unsigned char const * >(file->address)[
            off + i];
        if (c1 < c2) {
            return COMPARE_LESS;
        } else if (c1 > c2 || c2 == 0) {
            // ...the "|| c2 == 0" is for the odd case where name erroneously
            // contains NUL characters
            return COMPARE_GREATER;
        }
    }
    if (static_cast< sal_uInt64 >(nameLength) == min) {
        if (file->size - off == min) {
            throw FileFormatException(
                file->uri, "UNOIDL format: string misses trailing NUL");
        }
        return
            static_cast< unsigned char const * >(file->address)[off + min] == 0
            ? COMPARE_EQUAL : COMPARE_LESS;
    } else {
        return COMPARE_GREATER;
    }
}

sal_uInt32 findInMap(
    rtl::Reference< MappedFile > const & file, MapEntry const * mapBegin,
    sal_uInt32 mapSize, OUString const & name, sal_Int32 nameOffset,
    sal_Int32 nameLength)
{
    if (mapSize == 0) {
        return 0;
    }
    sal_uInt32 n = mapSize / 2;
    MapEntry const * p = mapBegin + n;
    switch (compare(file, name, nameOffset, nameLength, p)) {
    case COMPARE_LESS:
        return findInMap(file, mapBegin, n, name, nameOffset, nameLength);
    case COMPARE_GREATER:
        return findInMap(
            file, p + 1, mapSize - n - 1, name, nameOffset, nameLength);
    default: // COMPARE_EQUAL
        break;
    }
    sal_uInt32 off = mapBegin[n].data.getUnsigned32();
    if (off == 0) {
        throw FileFormatException(
            file->uri, "UNOIDL format: map entry data offset is null");
    }
    return off;
}

std::vector< OUString > readAnnotations(
    bool annotated, rtl::Reference< MappedFile > const & file,
    sal_uInt32 offset, sal_uInt32 * newOffset = 0)
{
    std::vector< OUString > ans;
    if (annotated) {
        sal_uInt32 n = file->read32(offset);
        offset += 4;
        for (sal_uInt32 i = 0; i != n; ++i) {
            ans.push_back(file->readIdxString(&offset));
        }
    }
    if (newOffset != 0) {
        *newOffset = offset;
    }
    return ans;
}

ConstantValue readConstant(
    rtl::Reference< MappedFile > const & file, sal_uInt32 offset,
    sal_uInt32 * newOffset = 0, bool * annotated = 0)
{
    assert(file.is());
    int v = file->read8(offset);
    int type = v & 0x7F;
    if (annotated != 0) {
        *annotated = (v & 0x80) != 0;
    }
    switch (type) {
    case 0: // BOOLEAN
        v = file->read8(offset + 1);
        if (newOffset != 0) {
            *newOffset = offset + 2;
        }
        switch (v) {
        case 0:
            return ConstantValue(false);
        case 1:
            return ConstantValue(true);
        default:
            throw FileFormatException(
                file->uri,
                ("UNOIDL format: bad boolean constant value "
                 + OUString::number(v)));
        }
    case 1: // BYTE
        if (newOffset != 0) {
            *newOffset = offset + 2;
        }
        return ConstantValue(static_cast< sal_Int8 >(file->read8(offset + 1)));
            //TODO: implementation-defined behavior of conversion from sal_uInt8
            // to sal_Int8 relies on two's complement representation
    case 2: // SHORT
        if (newOffset != 0) {
            *newOffset = offset + 3;
        }
        return ConstantValue(
            static_cast< sal_Int16 >(file->read16(offset + 1)));
            //TODO: implementation-defined behavior of conversion from
            // sal_uInt16 to sal_Int16 relies on two's complement representation
    case 3: // UNSIGNED SHORT
        if (newOffset != 0) {
            *newOffset = offset + 3;
        }
        return ConstantValue(file->read16(offset + 1));
    case 4: // LONG
        if (newOffset != 0) {
            *newOffset = offset + 5;
        }
        return ConstantValue(
            static_cast< sal_Int32 >(file->read32(offset + 1)));
            //TODO: implementation-defined behavior of conversion from
            // sal_uInt32 to sal_Int32 relies on two's complement representation
    case 5: // UNSIGNED LONG
        if (newOffset != 0) {
            *newOffset = offset + 5;
        }
        return ConstantValue(file->read32(offset + 1));
    case 6: // HYPER
        if (newOffset != 0) {
            *newOffset = offset + 9;
        }
        return ConstantValue(
            static_cast< sal_Int64 >(file->read64(offset + 1)));
            //TODO: implementation-defined behavior of conversion from
            // sal_uInt64 to sal_Int64 relies on two's complement representation
    case 7: // UNSIGNED HYPER
        if (newOffset != 0) {
            *newOffset = offset + 9;
        }
        return ConstantValue(file->read64(offset + 1));
    case 8: // FLOAT
        if (newOffset != 0) {
            *newOffset = offset + 5;
        }
        return ConstantValue(file->readIso60599Binary32(offset + 1));
    case 9: // DOUBLE
        if (newOffset != 0) {
            *newOffset = offset + 9;
        }
        return ConstantValue(file->readIso60599Binary64(offset + 1));
    default:
        throw FileFormatException(
            file->uri,
            "UNOIDL format: bad constant type byte " + OUString::number(v));
    }
}

rtl::Reference< Entity > readEntity(
    rtl::Reference< MappedFile > const & file, sal_uInt32 offset);

class UnoidlCursor: public MapCursor {
public:
    UnoidlCursor(
        rtl::Reference< MappedFile > file, MapEntry const * mapBegin,
        sal_uInt32 mapSize):
        file_(file), mapIndex_(mapBegin), mapEnd_(mapBegin + mapSize)
    {}

private:
    virtual ~UnoidlCursor() throw () {}

    virtual rtl::Reference< Entity > getNext(OUString * name);

    rtl::Reference< MappedFile > file_;
    MapEntry const * mapIndex_;
    MapEntry const * mapEnd_;
};

rtl::Reference< Entity > UnoidlCursor::getNext(OUString * name) {
    assert(name != 0);
    rtl::Reference< Entity > ent;
    if (mapIndex_ != mapEnd_) {
        *name = file_->readNulName(mapIndex_->name.getUnsigned32());
        ent = readEntity(file_, mapIndex_->data.getUnsigned32());
        ++mapIndex_;
    }
    return ent;
}

class UnoidlModuleEntity: public ModuleEntity {
public:
    UnoidlModuleEntity(
        rtl::Reference< MappedFile > const & file, sal_uInt32 mapOffset,
        sal_uInt32 mapSize):
        file_(file),
        mapBegin_(
            reinterpret_cast< MapEntry const * >(
                static_cast< char const * >(file_->address) + mapOffset)),
        mapSize_(mapSize)
    { assert(file.is()); }

private:
    virtual ~UnoidlModuleEntity() throw () {}

    virtual std::vector< OUString > getMemberNames() const;

    virtual rtl::Reference< MapCursor > createCursor() const
    { return new UnoidlCursor(file_, mapBegin_, mapSize_); }

    rtl::Reference< MappedFile > file_;
    MapEntry const * mapBegin_;
    sal_uInt32 mapSize_;
};

std::vector< OUString > UnoidlModuleEntity::getMemberNames() const {
    std::vector< OUString > names;
    for (sal_uInt32 i = 0; i != mapSize_; ++i) {
        names.push_back(file_->readNulName(mapBegin_[i].name.getUnsigned32()));
    }
    return names;
}

rtl::Reference< Entity > readEntity(
    rtl::Reference< MappedFile > const & file, sal_uInt32 offset)
{
    assert(file.is());
    int v = file->read8(offset);
    int type = v & 0x3F;
    bool published = (v & 0x80) != 0;
    bool annotated = (v & 0x40) != 0;
    bool flag = (v & 0x20) != 0;
    switch (type) {
    case 0: // module
        {
            if (v != 0) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: bad module type byte "
                     + OUString::number(v)));
            }
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri, "UNOIDL format: too many items in module");
            }
            if (offset + 5 + 8 * n > file->size) { //TODO: overflow
                throw FileFormatException(
                    file->uri,
                    "UNOIDL format: module map offset + size too large");
            }
            return new UnoidlModuleEntity(file, offset + 5, n);
        }
    case 1: // enum type
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri, "UNOIDL format: too many members of enum type");
            }
            offset += 5;
            std::vector< EnumTypeEntity::Member > mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                OUString memName(file->readIdxName(&offset));
                sal_Int32 memValue = static_cast< sal_Int32 >(
                    file->read32(offset));
                    //TODO: implementation-defined behavior of conversion from
                    // sal_uInt32 to sal_Int32 relies on two's complement
                    // representation
                offset += 4;
                mems.push_back(
                    EnumTypeEntity::Member(
                        memName, memValue,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            return new EnumTypeEntity(
                published, mems, readAnnotations(annotated, file, offset));
        }
    case 2: // plain struct type without base
    case 2 | 0x20: // plain struct type with base
        {
            ++offset;
            OUString base;
            if (flag) {
                base = file->readIdxName(&offset);
                if (base.isEmpty()) {
                    throw FileFormatException(
                        file->uri,
                        ("UNOIDL format: empty base type name of plain struct"
                         " type"));
                }
            }
            sal_uInt32 n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct members of plain struct"
                     " type"));
            }
            offset += 4;
            std::vector< PlainStructTypeEntity::Member > mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                OUString memName(file->readIdxName(&offset));
                OUString memType(file->readIdxName(&offset));
                mems.push_back(
                    PlainStructTypeEntity::Member(
                        memName, memType,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            return new PlainStructTypeEntity(
                published, base, mems,
                readAnnotations(annotated, file, offset));
        }
    case 3: // polymorphic struct type template
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many type parameters of polymorphic"
                     " struct type template"));
            }
            offset += 5;
            std::vector< OUString > params;
            for (sal_uInt32 i = 0; i != n; ++i) {
                params.push_back(file->readIdxName(&offset));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many members of polymorphic struct"
                     " type template"));
            }
            offset += 4;
            std::vector< PolymorphicStructTypeTemplateEntity::Member > mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                v = file->read8(offset);
                ++offset;
                OUString memName(file->readIdxName(&offset));
                OUString memType(file->readIdxName(&offset));
                if (v > 1) {
                    throw FileFormatException(
                        file->uri,
                        ("UNOIDL format: bad flags " + OUString::number(v)
                         + " for member " + memName
                         + " of polymorphic struct type template"));
                }
                mems.push_back(
                    PolymorphicStructTypeTemplateEntity::Member(
                        memName, memType, v == 1,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            return new PolymorphicStructTypeTemplateEntity(
                published, params, mems,
                readAnnotations(annotated, file, offset));
        }
    case 4: // exception type without base
    case 4 | 0x20: // exception type with base
        {
            ++offset;
            OUString base;
            if (flag) {
                base = file->readIdxName(&offset);
                if (base.isEmpty()) {
                    throw FileFormatException(
                        file->uri,
                        ("UNOIDL format: empty base type name of exception"
                         " type"));
                }
            }
            sal_uInt32 n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    "UNOIDL format: too many direct members of exception type");
            }
            offset += 4;
            std::vector< ExceptionTypeEntity::Member > mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                OUString memName(file->readIdxName(&offset));
                OUString memType(file->readIdxName(&offset));
                mems.push_back(
                    ExceptionTypeEntity::Member(
                        memName, memType,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            return new ExceptionTypeEntity(
                published, base, mems,
                readAnnotations(annotated, file, offset));
        }
    case 5: // interface type
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct mandatory bases of"
                     " interface type"));
            }
            offset += 5;
            std::vector< AnnotatedReference > mandBases;
            for (sal_uInt32 i = 0; i != n; ++i) {
                OUString base(file->readIdxName(&offset));
                mandBases.push_back(
                    AnnotatedReference(
                        base,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct optional bases of"
                     " interface type"));
            }
            offset += 4;
            std::vector< AnnotatedReference > optBases;
            for (sal_uInt32 i = 0; i != n; ++i) {
                OUString base(file->readIdxName(&offset));
                optBases.push_back(
                    AnnotatedReference(
                        base,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            sal_uInt32 nAttrs = file->read32(offset);
            if (nAttrs > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct attributes of interface"
                     " type"));
            }
            offset += 4;
            std::vector< InterfaceTypeEntity::Attribute > attrs;
            for (sal_uInt32 i = 0; i != nAttrs; ++i) {
                v = file->read8(offset);
                ++offset;
                OUString attrName(file->readIdxName(&offset));
                OUString attrType(file->readIdxName(&offset));
                if (v > 0x03) {
                    throw FileFormatException(
                        file->uri,
                        ("UNOIDL format: bad flags for direct attribute "
                         + attrName + " of interface type"));
                }
                std::vector< OUString > getExcs;
                sal_uInt32 m = file->read32(offset);
                if (m > SAL_MAX_INT32) {
                    throw FileFormatException(
                        file->uri,
                        ("UNOIDL format: too many getter exceptions for direct"
                         " attribute " + attrName + " of interface type"));
                }
                offset += 4;
                for (sal_uInt32 j = 0; j != m; ++j) {
                    getExcs.push_back(file->readIdxName(&offset));
                }
                std::vector< OUString > setExcs;
                if ((v & 0x02) == 0) {
                    m = file->read32(offset);
                    if (m > SAL_MAX_INT32) {
                        throw FileFormatException(
                            file->uri,
                            ("UNOIDL format: too many setter exceptions for"
                             " direct attribute " + attrName
                             + " of interface type"));
                    }
                    offset += 4;
                    for (sal_uInt32 j = 0; j != m; ++j) {
                        setExcs.push_back(file->readIdxName(&offset));
                    }
                }
                attrs.push_back(
                    InterfaceTypeEntity::Attribute(
                        attrName, attrType, (v & 0x01) != 0, (v & 0x02) != 0,
                        getExcs, setExcs,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            sal_uInt32 nMeths = file->read32(offset);
            if (nMeths > SAL_MAX_INT32 - nAttrs) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct attributes and methods of"
                     " interface type"));
            }
            offset += 4;
            std::vector< InterfaceTypeEntity::Method > meths;
            for (sal_uInt32 i = 0; i != nMeths; ++i) {
                OUString methName(file->readIdxName(&offset));
                OUString methType(file->readIdxName(&offset));
                sal_uInt32 m = file->read32(offset);
                if (m > SAL_MAX_INT32) {
                    throw FileFormatException(
                        file->uri,
                        ("UNOIDL format: too many parameters for method "
                         + methName + " of interface type"));
                }
                offset += 4;
                std::vector< InterfaceTypeEntity::Method::Parameter > params;
                for (sal_uInt32 j = 0; j != m; ++j) {
                    v = file->read8(offset);
                    ++offset;
                    OUString paramName(file->readIdxName(&offset));
                    OUString paramType(file->readIdxName(&offset));
                    InterfaceTypeEntity::Method::Parameter::Direction dir;
                    switch (v) {
                    case 0:
                        dir = InterfaceTypeEntity::Method::Parameter::
                            DIRECTION_IN;
                        break;
                    case 1:
                        dir = InterfaceTypeEntity::Method::Parameter::
                            DIRECTION_OUT;
                        break;
                    case 2:
                        dir = InterfaceTypeEntity::Method::Parameter::
                            DIRECTION_IN_OUT;
                        break;
                    default:
                        throw FileFormatException(
                            file->uri,
                            ("UNOIDL format: bad direction "
                             + OUString::number(v) + " of parameter "
                             + paramName + " for method " + methName
                             + " of interface type"));
                    }
                    params.push_back(
                        InterfaceTypeEntity::Method::Parameter(
                            paramName, paramType, dir));
                }
                std::vector< OUString > excs;
                m = file->read32(offset);
                if (m > SAL_MAX_INT32) {
                    throw FileFormatException(
                        file->uri,
                        ("UNOIDL format: too many exceptions for method "
                         + methName + " of interface type"));
                }
                offset += 4;
                for (sal_uInt32 j = 0; j != m; ++j) {
                    excs.push_back(file->readIdxName(&offset));
                }
                meths.push_back(
                    InterfaceTypeEntity::Method(
                        methName, methType, params, excs,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            return new InterfaceTypeEntity(
                published, mandBases, optBases, attrs, meths,
                readAnnotations(annotated, file, offset));
        }
    case 6: // typedef
        {
            ++offset;
            OUString base(file->readIdxName(&offset));
            return new TypedefEntity(
                published, base, readAnnotations(annotated, file, offset));
        }
    case 7: // constant group
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    "UNOIDL format: too many constants in constant group");
            }
            if (offset + 5 + 8 * n > file->size) { //TODO: overflow
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: constant group map offset + size too"
                     " large"));
            }
            MapEntry const * p = reinterpret_cast< MapEntry const * >(
                static_cast< char const * >(file->address) + offset + 5);
            std::vector< ConstantGroupEntity::Member > mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                sal_uInt32 off = p[i].data.getUnsigned32();
                bool ann;
                ConstantValue val(readConstant(file, off, &off, &ann));
                mems.push_back(
                    ConstantGroupEntity::Member(
                        file->readNulName(p[i].name.getUnsigned32()), val,
                        readAnnotations(ann, file, off)));
            }
            return new ConstantGroupEntity(
                published, mems,
                readAnnotations(annotated, file, offset + 5 + 8 * n));
        }
    case 8: // single-interface--based service without default constructor
    case 8 | 0x20: // single-interface--based service with default constructor
        {
            ++offset;
            OUString base(file->readIdxName(&offset));
            std::vector< SingleInterfaceBasedServiceEntity::Constructor > ctors;
            if (flag) {
                ctors.push_back(
                    SingleInterfaceBasedServiceEntity::Constructor());
            } else {
                sal_uInt32 n = file->read32(offset);
                if (n > SAL_MAX_INT32) {
                    throw FileFormatException(
                        file->uri,
                        ("UNOIDL format: too many constructors of"
                         " single-interface--based service"));
                }
                offset += 4;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    OUString ctorName(file->readIdxName(&offset));
                    sal_uInt32 m = file->read32(offset);
                    if (m > SAL_MAX_INT32) {
                        throw FileFormatException(
                            file->uri,
                            ("UNOIDL format: too many parameters for"
                             " constructor " + ctorName
                             + " of single-interface--based service"));
                    }
                    offset += 4;
                    std::vector<
                        SingleInterfaceBasedServiceEntity::Constructor::
                        Parameter > params;
                    for (sal_uInt32 j = 0; j != m; ++j) {
                        v = file->read8(offset);
                        ++offset;
                        OUString paramName(file->readIdxName(&offset));
                        OUString paramType(file->readIdxName(&offset));
                        bool rest;
                        switch (v) {
                        case 0:
                            rest = false;
                            break;
                        case 0x04:
                            rest = true;
                            break;
                        default:
                            throw FileFormatException(
                                file->uri,
                                ("UNOIDL format: bad mode "
                                 + OUString::number(v) + " of parameter "
                                 + paramName + " for constructor " + ctorName
                                 + " of single-interface--based service"));
                        }
                        params.push_back(
                            SingleInterfaceBasedServiceEntity::Constructor::
                            Parameter(
                                paramName, paramType, rest));
                    }
                    std::vector< OUString > excs;
                    m = file->read32(offset);
                    if (m > SAL_MAX_INT32) {
                        throw FileFormatException(
                            file->uri,
                            ("UNOIDL format: too many exceptions for"
                             " constructor " + ctorName
                             + " of single-interface--based service"));
                    }
                    offset += 4;
                    for (sal_uInt32 j = 0; j != m; ++j) {
                        excs.push_back(file->readIdxName(&offset));
                    }
                    ctors.push_back(
                        SingleInterfaceBasedServiceEntity::Constructor(
                            ctorName, params, excs,
                            readAnnotations(annotated, file, offset, &offset)));
                }
            }
            return new SingleInterfaceBasedServiceEntity(
                published, base, ctors,
                readAnnotations(annotated, file, offset));
        }
    case 9: // accumulation-based service
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct mandatory service bases of"
                     " accumulation-based service"));
            }
            offset += 5;
            std::vector< AnnotatedReference > mandServs;
            for (sal_uInt32 i = 0; i != n; ++i) {
                OUString base(file->readIdxName(&offset));
                mandServs.push_back(
                    AnnotatedReference(
                        base,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct optional service bases of"
                     " accumulation-based service"));
            }
            offset += 4;
            std::vector< AnnotatedReference > optServs;
            for (sal_uInt32 i = 0; i != n; ++i) {
                OUString base(file->readIdxName(&offset));
                optServs.push_back(
                    AnnotatedReference(
                        base,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct mandatory interface bases"
                     " of accumulation-based service"));
            }
            offset += 4;
            std::vector< AnnotatedReference > mandIfcs;
            for (sal_uInt32 i = 0; i != n; ++i) {
                OUString base(file->readIdxName(&offset));
                mandIfcs.push_back(
                    AnnotatedReference(
                        base,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct optional interface bases"
                     " of accumulation-based service"));
            }
            offset += 4;
            std::vector< AnnotatedReference > optIfcs;
            for (sal_uInt32 i = 0; i != n; ++i) {
                OUString base(file->readIdxName(&offset));
                optIfcs.push_back(
                    AnnotatedReference(
                        base,
                        readAnnotations(annotated, file, offset, &offset)));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw FileFormatException(
                    file->uri,
                    ("UNOIDL format: too many direct properties of"
                     " accumulation-based service"));
            }
            offset += 4;
            std::vector< AccumulationBasedServiceEntity::Property > props;
            for (sal_uInt32 i = 0; i != n; ++i) {
                sal_uInt16 attrs = file->read16(offset);
                offset += 2;
                OUString propName(file->readIdxName(&offset));
                OUString propType(file->readIdxName(&offset));
                if (attrs > 0x01FF) { // see css.beans.PropertyAttribute
                    throw FileFormatException(
                        file->uri,
                        ("UNOIDL format: bad mode " + OUString::number(v)
                         + " of property " + propName
                         + " for accumulation-based servcie"));
                }
                props.push_back(
                    AccumulationBasedServiceEntity::Property(
                        propName, propType,
                        static_cast<
                            AccumulationBasedServiceEntity::Property::
                            Attributes >(
                                attrs),
                        readAnnotations(annotated, file, offset, &offset)));
            }
            return new AccumulationBasedServiceEntity(
                published, mandServs, optServs, mandIfcs, optIfcs, props,
                readAnnotations(annotated, file, offset));
        }
    case 10: // interface-based singleton
        {
            ++offset;
            OUString base(file->readIdxName(&offset));
            return new InterfaceBasedSingletonEntity(
                published, base, readAnnotations(annotated, file, offset));
        }
    case 11: // service-based singleton
        {
            ++offset;
            OUString base(file->readIdxName(&offset));
            return new ServiceBasedSingletonEntity(
                published, base, readAnnotations(annotated, file, offset));
        }
    default:
        throw FileFormatException(
            file->uri, "UNOIDL format: bad type byte " + OUString::number(v));
    }
}

}

UnoidlProvider::UnoidlProvider(OUString const & uri): file_(new MappedFile(uri))
{
    if (file_->size < 8 || std::memcmp(file_->address, "UNOIDL\xFF\0", 8) != 0)
    {
        throw FileFormatException(
            file_->uri,
            "UNOIDL format: does not begin with magic UNOIDL\\xFF and version"
            " 0");
    }
    sal_uInt32 off = file_->read32(8);
    mapSize_ = file_->read32(12);
    if (off + 8 * mapSize_ > file_->size) { //TODO: overflow
        throw FileFormatException(
            file_->uri, "UNOIDL format: root map offset + size too large");
    }
    mapBegin_ = reinterpret_cast< MapEntry const * >(
        static_cast< char const * >(file_->address) + off);
}

rtl::Reference< MapCursor > UnoidlProvider::createRootCursor() const {
    return new UnoidlCursor(file_, mapBegin_, mapSize_);
}

rtl::Reference< Entity > UnoidlProvider::findEntity(OUString const & name) const
{
    MapEntry const * mapBegin = mapBegin_;
    sal_uInt32 mapSize = mapSize_;
    bool cgroup = false;
    for (sal_Int32 i = 0;;) {
        sal_Int32 j = name.indexOf('.', i);
        if (j == -1) {
            j = name.getLength();
        }
        sal_Int32 off = findInMap(file_, mapBegin, mapSize, name, i, j - i);
        if (off == 0) {
            return rtl::Reference< Entity >();
        }
        if (j == name.getLength()) {
            return cgroup ? rtl::Reference< Entity >() : readEntity(file_, off);
        }
        if (cgroup) {
            return rtl::Reference< Entity >();
                //TODO: throw an exception instead here, where the segments of a
                // constant's name are a prefix of the requested name's
                // segments?
        }
        int v = file_->read8(off);
        if (v != 0) { // module
            if ((v & 0x3F) == 7) { // constant group
                cgroup = true;
            } else {
                return rtl::Reference< Entity >();
                    //TODO: throw an exception instead here, where the segments
                    // of a non-module, non-constant-group entity's name are a
                    // prefix of the requested name's segments?
            }
        }
        mapSize = file_->read32(off + 1);
        if (8 * mapSize > file_->size - off - 5) { //TODO: overflow
            throw FileFormatException(
                file_->uri, "UNOIDL format: map offset + size too large");
        }
        mapBegin = reinterpret_cast< MapEntry const * >(
            static_cast< char const * >(file_->address) + off + 5);
        i = j + 1;
    }
}

UnoidlProvider::~UnoidlProvider() throw () {}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
