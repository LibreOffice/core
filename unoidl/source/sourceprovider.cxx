/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cerrno>
#include <map>
#include <new>

#include "osl/file.h"
#include "osl/thread.h"
#include "rtl/character.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "unoidl/sourceprovider.hxx"
#include "unoidl/unoidl.hxx"

#include "sourceprovider-parser-requires.hxx"
#include "sourceprovider-parser.hxx"
#include "sourceprovider-scanner.hxx"

namespace unoidl {

namespace {

rtl::Reference<Entity> parse(
    rtl::Reference<Manager> const & manager, OUString const & name,
    OUString const & uri, void const * address, sal_uInt64 size)
{
    detail::SourceProviderScannerData data(manager, address, size);
    yyscan_t yyscanner;
    if (yylex_init_extra(&data, &yyscanner) != 0) {
        // Checking errno for the specific EINVAL, ENOMEM documented for
        // yylex_init_extra would not work as those values are not defined by
        // the C++ Standard:
        int e = errno;
        throw FileFormatException(
            uri, "yylex_init_extra failed with errno " + OUString::number(e));
    }
    int e = yyparse(yyscanner);
    yylex_destroy(yyscanner);
    switch (e) {
    case 0:
        {
            std::map<OUString, detail::SourceProviderEntity>::const_iterator i(
                data.entities.find(name));
            return i == data.entities.end()
                ? rtl::Reference<Entity>() : i->second.entity;
        }
    default:
        assert(false);
        // fall through
    case 1:
        throw FileFormatException(
            uri,
            ("cannot parse"
             + (data.errorLine == 0
                ? OUString() : " line " + OUString::number(data.errorLine))
             + (data.parserError.isEmpty()
                ? OUString()
                : (", "
                   + OStringToOUString(
                       data.parserError, osl_getThreadTextEncoding())))
             + (data.errorMessage.isEmpty()
                ? OUString() : ": \"" + data.errorMessage + "\"")));
    case 2:
        throw std::bad_alloc();
    }
}

class Cursor: public MapCursor {
public:
    Cursor() {}

private:
    virtual ~Cursor() throw () {}

    virtual rtl::Reference<Entity> getNext(OUString *)
    { return rtl::Reference<Entity>(); } //TODO
};

}

SourceProvider::SourceProvider(
    rtl::Reference<Manager> const & manager, OUString const & uri):
    manager_(manager), uri_(uri.endsWith("/") ? uri : uri + "/")
{}

rtl::Reference<MapCursor> SourceProvider::createRootCursor() const {
    return new Cursor;
}

rtl::Reference<Entity> SourceProvider::findEntity(OUString const & name) const {
    std::map< OUString, rtl::Reference<Entity> >::iterator ci(
        cache_.find(name));
    if (ci != cache_.end()) {
        return ci->second;
    }
    // Match name against
    //   name ::= identifier ("." identifier)*
    //   identifier ::= upper-blocks | lower-block
    //   upper-blocks ::= upper ("_"? alnum)*
    //   lower-block :== lower alnum*
    //   alnum ::= digit | upper | lower
    //   digit ::= "0"--"9"
    //   upper ::= "A"--"Z"
    //   lower ::= "a"--"z"
    OUStringBuffer buf(name);
    sal_Int32 start = 0;
    sal_Int32 i = 0;
    for (; i != name.getLength(); ++i) {
        sal_Unicode c = name[i];
        if (c == '.') {
            assert(i == start || i != 0);
            if (i == start || name[i - 1] == '_') {
                throw FileFormatException( //TODO
                    "", "Illegal UNOIDL identifier \"" + name + "\"");
            }
            buf[i] = '/';
            start = i + 1;
        } else if (c == '_') {
            assert(i == start || i != 0);
            if (i == start || name[i - 1] == '_'
                || !rtl::isAsciiUpperCase(name[start]))
            {
                throw FileFormatException( //TODO
                    "", "Illegal UNOIDL identifier \"" + name + "\"");
            }
        } else if (rtl::isAsciiDigit(c)) {
            if (i == start) {
                throw FileFormatException( //TODO
                    "", "Illegal UNOIDL identifier \"" + name + "\"");
            }
        } else if (!rtl::isAsciiAlpha(c)) {
            throw FileFormatException( //TODO
                "", "Illegal UNOIDL identifier \"" + name + "\"");
        }
    }
    if (i == start) {
        throw FileFormatException( //TODO
            "", "Illegal UNOIDL identifier \"" + name + "\"");
    }
    OUString uri(uri_ + buf.makeStringAndClear() + ".idl");
    oslFileHandle handle;
    oslFileError e = osl_openFile(uri.pData, &handle, osl_File_OpenFlag_Read);
    switch (e) {
    case osl_File_E_None:
        break;
    case osl_File_E_NOENT:
        cache_.insert(
            std::map< OUString, rtl::Reference<Entity> >::value_type(
                name, rtl::Reference<Entity>()));
        return rtl::Reference<Entity>();
    default:
        throw FileFormatException(uri, "cannot open: " + OUString::number(e));
    }
    sal_uInt64 size;
    e = osl_getFileSize(handle, &size);
    if (e != osl_File_E_None) {
        oslFileError e2 = osl_closeFile(handle);
        SAL_WARN_IF(
            e2 != osl_File_E_None, "unoidl",
            "cannot close " << uri << ": " << +e2);
        throw FileFormatException(
            uri, "cannot get size: " + OUString::number(e));
    }
    void * address;
    e = osl_mapFile(handle, &address, size, 0, osl_File_MapFlag_RandomAccess);
    if (e != osl_File_E_None) {
        oslFileError e2 = osl_closeFile(handle);
        SAL_WARN_IF(
            e2 != osl_File_E_None, "unoidl",
            "cannot close " << uri << ": " << +e2);
        throw FileFormatException(uri, "cannot mmap: " + OUString::number(e));
    }
    rtl::Reference<Entity> ent;
    try {
        ent = parse(manager_, name, uri, address, size);
    } catch (...) {
        e = osl_unmapMappedFile(handle, address, size);
        SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot unmap: " << +e);
        e = osl_closeFile(handle);
        SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot close: " << +e);
        throw;
    }
    e = osl_unmapMappedFile(handle, address, size);
    SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot unmap: " << +e);
    e = osl_closeFile(handle);
    SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot close: " << +e);
    cache_.insert(
        std::map< OUString, rtl::Reference<Entity> >::value_type(name, ent));
    return ent;
}

SourceProvider::~SourceProvider() throw () {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
