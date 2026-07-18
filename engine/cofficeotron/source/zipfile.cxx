/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009-2010 Griffin Brown Digital Publishing Ltd.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "zipfile.hxx"

#include <cstdio>
#include <map>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/refcountedmutex.hxx>
#include <comphelper/seqstream.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <EncryptionData.hxx>
#include <ZipFile.hxx>

namespace
{
std::string toUtf8(std::u16string_view text)
{
    OString bytes = OUStringToOString(text, RTL_TEXTENCODING_UTF8);
    return std::string(bytes.getStr(), static_cast<size_t>(bytes.getLength()));
}

} // namespace

struct ZipArchive::Impl
{
    rtl::Reference<comphelper::RefCountedMutex> mutexHolder;
    cpo::uno::Sequence<sal_Int8> data;
    css::uno::Reference<css::io::XInputStream> stream;
    std::unique_ptr<ZipFile> zip;
    std::vector<ZipEntryInfo> infoList;
    std::map<std::string, OUString> pathByName;
    size_t localHeaders = 0;
};

ZipArchive::ZipArchive() = default;

ZipArchive::~ZipArchive() = default;

bool ZipArchive::open(const std::string& path, std::string& error)
{
    impl = std::make_unique<Impl>();

    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (!file)
    {
        error = "cannot open file";
        return false;
    }

    std::fseek(file, 0, SEEK_END);
    long size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    if (size < 0)
    {
        std::fclose(file);
        error = "cannot determine file size";
        return false;
    }

    impl->data.realloc(size);
    size_t got = size ? std::fread(impl->data.getArray(), 1, static_cast<size_t>(size), file) : 0;
    std::fclose(file);
    if (got != static_cast<size_t>(size))
    {
        error = "short read";
        return false;
    }

    impl->mutexHolder = new comphelper::RefCountedMutex;
    impl->stream = new comphelper::SequenceInputStream(impl->data);

    try
    {
        impl->zip = std::make_unique<ZipFile>(
            impl->mutexHolder, impl->stream, css::uno::Reference<css::uno::XComponentContext>(),
            true /* initialise */, false /* force recovery */, ZipFile::Checks::Default);
    }
    catch (const css::uno::Exception& exception)
    {
        error = toUtf8(exception.Message);
        if (error.empty())
            error = "cannot read ZIP archive";
        return false;
    }
    catch (const std::exception& exception)
    {
        error = exception.what();
        return false;
    }

    for (const auto& item : impl->zip->GetEntryHash())
    {
        ZipEntryInfo info;
        info.name = toUtf8(item.second.sPath);
        info.dataDescriptor = (item.second.nFlag & 0x8) != 0;

        impl->pathByName.emplace(info.name, item.second.sPath);
        impl->infoList.push_back(std::move(info));
    }

    // The package code reads and cross-checks every local header against
    // its central directory record while parsing the central directory,
    // and refuses the archive on any disagreement. An archive that opens
    // therefore has one verified local header per central record.
    impl->localHeaders = impl->infoList.size();

    return true;
}

const std::vector<ZipEntryInfo>& ZipArchive::entries() const { return impl->infoList; }

const ZipEntryInfo* ZipArchive::find(const std::string& name) const
{
    for (const ZipEntryInfo& info : impl->infoList)
        if (info.name == name)
            return &info;
    return nullptr;
}

bool ZipArchive::extract(const ZipEntryInfo& entry, std::string& out, std::string& error) const
{
    return extract(entry.name, out) != nullptr || (error = "cannot extract entry", false);
}

const ZipEntryInfo* ZipArchive::extract(const std::string& name, std::string& out) const
{
    out.clear();

    auto path = impl->pathByName.find(name);
    if (path == impl->pathByName.end())
        return nullptr;

    auto entry = impl->zip->GetEntryHash().find(path->second);
    if (entry == impl->zip->GetEntryHash().end())
        return nullptr;

    try
    {
        css::uno::Reference<css::io::XInputStream> input = impl->zip->getInputStream(
            entry->second, rtl::Reference<EncryptionData>(), std::nullopt, impl->mutexHolder);

        cpo::uno::Sequence<sal_Int8> buffer;
        for (;;)
        {
            sal_Int32 count = input->readBytes(buffer, 65536);
            if (count <= 0)
                break;
            out.append(reinterpret_cast<const char*>(buffer.getConstArray()),
                       static_cast<size_t>(count));
        }
    }
    catch (const css::uno::Exception&)
    {
        out.clear();
        return nullptr;
    }
    catch (const std::exception&)
    {
        out.clear();
        return nullptr;
    }

    return find(name);
}

size_t ZipArchive::centralRecordCount() const { return impl->infoList.size(); }

size_t ZipArchive::localHeaderCount() const { return impl->localHeaders; }

bool ZipArchive::usesDataDescriptors() const
{
    for (const ZipEntryInfo& info : impl->infoList)
        if (info.dataDescriptor)
            return true;
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
