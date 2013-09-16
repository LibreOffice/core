/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <map>
#include <vector>

#include "osl/file.h"
#include "osl/file.hxx"
#include "rtl/character.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "unoidl/unoidl.hxx"

#include "sourceprovider-parser-requires.hxx"
#include "sourceprovider-parser.hxx"
#include "sourceprovider-scanner.hxx"
#include "sourceprovider.hxx"

namespace unoidl { namespace detail {

namespace {

class Cursor: public MapCursor {
public:
    Cursor() {}

private:
    virtual ~Cursor() throw () {}

    virtual rtl::Reference<Entity> getNext(OUString *)
    { return rtl::Reference<Entity>(); } //TODO
};

class SourceModuleEntity: public ModuleEntity {
public:
    SourceModuleEntity() {}

private:
    virtual ~SourceModuleEntity() throw () {}

    virtual std::vector<OUString> getMemberNames() const
    { return std::vector<OUString>(); } //TODO

    virtual rtl::Reference< MapCursor > createCursor() const
    { return new Cursor; }
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
    OUString uri(uri_ + buf.makeStringAndClear());
    rtl::Reference<Entity> ent;
    osl::DirectoryItem item;
    osl::FileStatus status(osl_FileStatus_Mask_Type);
    if (osl::DirectoryItem::get(uri, item) == osl::FileBase::E_None
        && item.getFileStatus(status) == osl::FileBase::E_None
        && status.getFileType() == osl::FileStatus::Directory)
    {
        ent = new SourceModuleEntity;
    } else {
        uri += ".idl";
        SourceProviderScannerData data(manager_);
        if (parse(uri, &data)) {
            std::map<OUString, SourceProviderEntity>::const_iterator i(
                data.entities.find(name));
            if (i != data.entities.end()) {
                ent = i->second.entity;
            }
            SAL_WARN_IF(
                !ent.is(), "unoidl",
                "<" << uri << "> does not define entity " << name);
        }
    }
    cache_.insert(
        std::map< OUString, rtl::Reference<Entity> >::value_type(name, ent));
    return ent;
}

SourceProvider::~SourceProvider() throw () {}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
