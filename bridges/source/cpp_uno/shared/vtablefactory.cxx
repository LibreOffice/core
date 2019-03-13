/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <vtablefactory.hxx>

#include <vtables.hxx>

#include <osl/thread.h>
#include <osl/security.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <typelib/typedescription.hxx>

#include <memory>
#include <new>
#include <unordered_map>
#include <vector>

#if defined SAL_UNX
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#elif defined _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#error Unsupported platform
#endif

#if defined USE_DOUBLE_MMAP
#include <fcntl.h>
#endif

using bridges::cpp_uno::shared::VtableFactory;

namespace {

extern "C" void * allocExec(
    SAL_UNUSED_PARAMETER rtl_arena_type *, sal_Size * size)
{
    std::size_t pagesize;
#if defined SAL_UNX
#if defined FREEBSD || defined NETBSD || defined OPENBSD || defined DRAGONFLY || defined HAIKU
    pagesize = getpagesize();
#else
    pagesize = sysconf(_SC_PAGESIZE);
#endif
#elif defined _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    pagesize = info.dwPageSize;
#else
#error Unsupported platform
#endif
    std::size_t n = (*size + (pagesize - 1)) & ~(pagesize - 1);
    void * p;
#if defined SAL_UNX
    p = mmap(
        nullptr, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1,
        0);
    if (p == MAP_FAILED) {
        p = nullptr;
    }
    else if (mprotect (p, n, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
    {
        munmap (p, n);
        p = nullptr;
    }
#elif defined _WIN32
    p = VirtualAlloc(nullptr, n, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#endif
    if (p != nullptr) {
        *size = n;
    }
    return p;
}

extern "C" void freeExec(
    SAL_UNUSED_PARAMETER rtl_arena_type *, void * address, sal_Size size)
{
#if defined SAL_UNX
    munmap(address, size);
#elif defined _WIN32
    (void) size; // unused
    VirtualFree(address, 0, MEM_RELEASE);
#endif
}

}

class VtableFactory::GuardedBlocks:
    public std::vector<Block>
{
public:
    GuardedBlocks(const GuardedBlocks&) = delete;
    const GuardedBlocks& operator=(const GuardedBlocks&) = delete;

    explicit GuardedBlocks(VtableFactory const & factory):
        m_factory(factory), m_guarded(true) {}

    ~GuardedBlocks();

    void unguard() { m_guarded = false; }

private:
    VtableFactory const & m_factory;
    bool m_guarded;
};

VtableFactory::GuardedBlocks::~GuardedBlocks() {
    if (m_guarded) {
        for (iterator i(begin()); i != end(); ++i) {
            m_factory.freeBlock(*i);
        }
    }
}

class VtableFactory::BaseOffset {
public:
    explicit BaseOffset(typelib_InterfaceTypeDescription * type) { calculate(type, 0); }

    sal_Int32 getFunctionOffset(OUString const & name) const
    { return m_map.find(name)->second; }

private:
    sal_Int32 calculate(
        typelib_InterfaceTypeDescription * type, sal_Int32 offset);

    std::unordered_map< OUString, sal_Int32 > m_map;
};

sal_Int32 VtableFactory::BaseOffset::calculate(
    typelib_InterfaceTypeDescription * type, sal_Int32 offset)
{
    OUString name(type->aBase.pTypeName);
    if (m_map.find(name) == m_map.end()) {
        for (sal_Int32 i = 0; i < type->nBaseTypes; ++i) {
            offset = calculate(type->ppBaseTypes[i], offset);
        }
        m_map.insert({name, offset});
        typelib_typedescription_complete(
            reinterpret_cast< typelib_TypeDescription ** >(&type));
        offset += bridges::cpp_uno::shared::getLocalFunctions(type);
    }
    return offset;
}

VtableFactory::VtableFactory(): m_arena(
    rtl_arena_create(
        "bridges::cpp_uno::shared::VtableFactory",
        sizeof (void *), // to satisfy alignment requirements
        0, nullptr, allocExec, freeExec, 0))
{
    if (m_arena == nullptr) {
        throw std::bad_alloc();
    }
}

VtableFactory::~VtableFactory() {
    {
        osl::MutexGuard guard(m_mutex);
        for (auto& rEntry : m_map) {
            for (sal_Int32 j = 0; j < rEntry.second.count; ++j) {
                freeBlock(rEntry.second.blocks[j]);
            }
            delete[] rEntry.second.blocks;
        }
    }
    rtl_arena_destroy(m_arena);
}

VtableFactory::Vtables VtableFactory::getVtables(
    typelib_InterfaceTypeDescription * type)
{
    OUString name(type->aBase.pTypeName);
    osl::MutexGuard guard(m_mutex);
    Map::iterator i(m_map.find(name));
    if (i == m_map.end()) {
        GuardedBlocks blocks(*this);
        createVtables(blocks, BaseOffset(type), type, 0, type, true);
        Vtables vtables;
        assert(blocks.size() <= SAL_MAX_INT32);
        vtables.count = static_cast< sal_Int32 >(blocks.size());
        std::unique_ptr< Block[] > guardedBlocks(
            new Block[vtables.count]);
        vtables.blocks = guardedBlocks.get();
        for (sal_Int32 j = 0; j < vtables.count; ++j) {
            vtables.blocks[j] = blocks[j];
        }
        i = m_map.emplace(name, vtables).first;
        guardedBlocks.release();
        blocks.unguard();
    }
    return i->second;
}

#ifdef USE_DOUBLE_MMAP
bool VtableFactory::createBlock(Block &block, sal_Int32 slotCount) const
{
    std::size_t size = getBlockSize(slotCount);
    std::size_t pagesize = sysconf(_SC_PAGESIZE);
    block.size = (size + (pagesize - 1)) & ~(pagesize - 1);
    block.fd = -1;

    // Try non-doublemmaped allocation first:
    block.start = block.exec = rtl_arena_alloc(m_arena, &block.size);
    if (block.start != nullptr) {
        return true;
    }

    osl::Security aSecurity;
    OUString strDirectory;
    OUString strURLDirectory;
    if (aSecurity.getHomeDir(strURLDirectory))
        osl::File::getSystemPathFromFileURL(strURLDirectory, strDirectory);

    for (int i = strDirectory.isEmpty() ? 1 : 0; i < 2; ++i)
    {
        if (strDirectory.isEmpty())
            strDirectory = "/tmp";

        strDirectory += "/.execoooXXXXXX";
        OString aTmpName = OUStringToOString(strDirectory, osl_getThreadTextEncoding());
        std::unique_ptr<char[]> tmpfname(new char[aTmpName.getLength()+1]);
        strncpy(tmpfname.get(), aTmpName.getStr(), aTmpName.getLength()+1);
        // coverity[secure_temp] - https://communities.coverity.com/thread/3179
        if ((block.fd = mkstemp(tmpfname.get())) == -1)
            fprintf(stderr, "mkstemp(\"%s\") failed: %s\n", tmpfname.get(), strerror(errno));
        if (block.fd == -1)
        {
            break;
        }
        unlink(tmpfname.get());
        tmpfname.reset();
#if defined(HAVE_POSIX_FALLOCATE)
        int err = posix_fallocate(block.fd, 0, block.size);
#else
        int err = ftruncate(block.fd, block.size);
#endif
        if (err != 0)
        {
#if defined(HAVE_POSIX_FALLOCATE)
            SAL_WARN("bridges", "posix_fallocate failed with code " << err);
#else
            SAL_WARN("bridges", "truncation of executable memory area failed with code " << err);
#endif
            close(block.fd);
            block.fd = -1;
            break;
        }
        block.start = mmap(nullptr, block.size, PROT_READ | PROT_WRITE, MAP_SHARED, block.fd, 0);
        if (block.start== MAP_FAILED) {
            block.start = nullptr;
        }
        block.exec = mmap(nullptr, block.size, PROT_READ | PROT_EXEC, MAP_SHARED, block.fd, 0);
        if (block.exec == MAP_FAILED) {
           block.exec = nullptr;
        }

        //All good
        if (block.start && block.exec && block.fd != -1)
            break;

        freeBlock(block);

        strDirectory.clear();
    }
    return (block.start != nullptr && block.exec != nullptr);
}

void VtableFactory::freeBlock(Block const & block) const {
    //if the double-map failed we were allocated on the arena
    if (block.fd == -1 && block.start == block.exec && block.start != nullptr)
        rtl_arena_free(m_arena, block.start, block.size);
    else
    {
        if (block.start) munmap(block.start, block.size);
        if (block.exec) munmap(block.exec, block.size);
        if (block.fd != -1) close(block.fd);
    }
}
#else
bool VtableFactory::createBlock(Block &block, sal_Int32 slotCount) const
{
    block.size = getBlockSize(slotCount);
    block.start = rtl_arena_alloc(m_arena, &block.size);
    return block.start != nullptr;
}

void VtableFactory::freeBlock(Block const & block) const {
    rtl_arena_free(m_arena, block.start, block.size);
}
#endif

sal_Int32 VtableFactory::createVtables(
    GuardedBlocks & blocks, BaseOffset const & baseOffset,
    typelib_InterfaceTypeDescription * type, sal_Int32 vtableNumber,
    typelib_InterfaceTypeDescription * mostDerived, bool includePrimary) const
{
    if (includePrimary) {
        sal_Int32 slotCount
            = bridges::cpp_uno::shared::getPrimaryFunctions(type);
        Block block;
        if (!createBlock(block, slotCount)) {
            throw std::bad_alloc();
        }
        try {
            Slot * slots = initializeBlock(
                block.start, slotCount, vtableNumber, mostDerived);
            unsigned char * codeBegin =
                reinterpret_cast< unsigned char * >(slots);
            unsigned char * code = codeBegin;
            sal_Int32 vtableOffset = blocks.size() * sizeof (Slot *);
            for (typelib_InterfaceTypeDescription const * type2 = type;
                 type2 != nullptr; type2 = type2->pBaseTypeDescription)
            {
                code = addLocalFunctions(
                    &slots, code,
#ifdef USE_DOUBLE_MMAP
                    sal_uIntPtr(block.exec) - sal_uIntPtr(block.start),
#endif
                    type2,
                    baseOffset.getFunctionOffset(type2->aBase.pTypeName),
                    bridges::cpp_uno::shared::getLocalFunctions(type2),
                    vtableOffset);
            }
            flushCode(codeBegin, code);
#ifdef USE_DOUBLE_MMAP
        //Finished generating block, swap writable pointer with executable
        //pointer
            std::swap(block.start, block.exec);
#endif
            blocks.push_back(block);
        } catch (...) {
            freeBlock(block);
            throw;
        }
    }
    for (sal_Int32 i = 0; i < type->nBaseTypes; ++i) {
        vtableNumber = createVtables(
            blocks, baseOffset, type->ppBaseTypes[i],
            vtableNumber + (i == 0 ? 0 : 1), mostDerived, i != 0);
    }
    return vtableNumber;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
