/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#if defined OS2
#define INCL_DOS
#define INCL_DOSMISC
#endif

#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "guardedarray.hxx"

#include "bridges/cpp_uno/shared/vtables.hxx"

#include "osl/thread.h"
#include "osl/security.hxx"
#include "osl/file.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/alloc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"

#include <boost/unordered_map.hpp>
#include <new>
#include <vector>

#if defined SAL_UNX
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#elif defined SAL_W32
#define WIN32_LEAN_AND_MEAN
#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#elif defined SAL_OS2
#define INCL_DOS
#define INCL_DOSMISC
#include <os2.h>
#else
#error Unsupported platform
#endif

using bridges::cpp_uno::shared::VtableFactory;

namespace {

extern "C" void * SAL_CALL allocExec(rtl_arena_type *, sal_Size * size) {
    sal_Size pagesize;
#if defined SAL_UNX
#if defined FREEBSD || defined NETBSD || defined OPENBSD || defined DRAGONFLY
    pagesize = getpagesize();
#else
    pagesize = sysconf(_SC_PAGESIZE);
#endif
#elif defined SAL_W32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    pagesize = info.dwPageSize;
#elif defined(SAL_OS2)
    ULONG ulPageSize;
    DosQuerySysInfo(QSV_PAGE_SIZE, QSV_PAGE_SIZE, &ulPageSize, sizeof(ULONG));
    pagesize = (sal_Size)ulPageSize;
#else
#error Unsupported platform
#endif
    sal_Size n = (*size + (pagesize - 1)) & ~(pagesize - 1);
    void * p;
#if defined SAL_UNX
    p = mmap(
        0, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1,
        0);
    if (p == MAP_FAILED) {
        p = 0;
    }
    else if (mprotect (static_cast<char*>(p), n, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
    {
        munmap (static_cast<char*>(p), n);
        p = 0;
    }
#elif defined SAL_W32
    p = VirtualAlloc(0, n, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#elif defined(SAL_OS2)
    p = 0;
    DosAllocMem( &p, n, PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_ANY);
#endif
    if (p != 0) {
        *size = n;
    }
    return p;
}

extern "C" void SAL_CALL freeExec(
    rtl_arena_type *, void * address, sal_Size size)
{
#if defined SAL_UNX
    munmap(static_cast< char * >(address), size);
#elif defined SAL_W32
    (void) size; // unused
    VirtualFree(address, 0, MEM_RELEASE);
#elif defined(SAL_OS2)
    (void) DosFreeMem( address);
#endif
}

}

class VtableFactory::GuardedBlocks: public std::vector< Block > {
public:
    GuardedBlocks(VtableFactory const & factory):
        m_factory(factory), m_guarded(true) {}

    ~GuardedBlocks();

    void unguard() { m_guarded = false; }

private:
    GuardedBlocks(GuardedBlocks &); // not implemented
    void operator =(GuardedBlocks); // not implemented

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
    BaseOffset(typelib_InterfaceTypeDescription * type) { calculate(type, 0); }

    sal_Int32 getFunctionOffset(rtl::OUString const & name) const
    { return m_map.find(name)->second; }

private:
    sal_Int32 calculate(
        typelib_InterfaceTypeDescription * type, sal_Int32 offset);

    typedef boost::unordered_map< rtl::OUString, sal_Int32, rtl::OUStringHash > Map;

    Map m_map;
};

sal_Int32 VtableFactory::BaseOffset::calculate(
    typelib_InterfaceTypeDescription * type, sal_Int32 offset)
{
    rtl::OUString name(type->aBase.pTypeName);
    if (m_map.find(name) == m_map.end()) {
        for (sal_Int32 i = 0; i < type->nBaseTypes; ++i) {
            offset = calculate(type->ppBaseTypes[i], offset);
        }
        m_map.insert(Map::value_type(name, offset));
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
        0, reinterpret_cast< rtl_arena_type * >(-1), allocExec, freeExec, 0))
{
    if (m_arena == 0) {
        throw std::bad_alloc();
    }
}

VtableFactory::~VtableFactory() {
    {
        osl::MutexGuard guard(m_mutex);
        for (Map::iterator i(m_map.begin()); i != m_map.end(); ++i) {
            for (sal_Int32 j = 0; j < i->second.count; ++j) {
                freeBlock(i->second.blocks[j]);
            }
            delete[] i->second.blocks;
        }
    }
    rtl_arena_destroy(m_arena);
}

VtableFactory::Vtables VtableFactory::getVtables(
    typelib_InterfaceTypeDescription * type)
{
    rtl::OUString name(type->aBase.pTypeName);
    osl::MutexGuard guard(m_mutex);
    Map::iterator i(m_map.find(name));
    if (i == m_map.end()) {
        GuardedBlocks blocks(*this);
        createVtables(blocks, BaseOffset(type), type, true);
        Vtables vtables;
        OSL_ASSERT(blocks.size() <= SAL_MAX_INT32);
        vtables.count = static_cast< sal_Int32 >(blocks.size());
        bridges::cpp_uno::shared::GuardedArray< Block > guardedBlocks(
            new Block[vtables.count]);
        vtables.blocks = guardedBlocks.get();
        for (sal_Int32 j = 0; j < vtables.count; ++j) {
            vtables.blocks[j] = blocks[j];
        }
        i = m_map.insert(Map::value_type(name, vtables)).first;
        guardedBlocks.release();
        blocks.unguard();
    }
    return i->second;
}

#ifdef USE_DOUBLE_MMAP
bool VtableFactory::createBlock(Block &block, sal_Int32 slotCount) const
{
    sal_Size size = getBlockSize(slotCount);
    sal_Size pagesize = sysconf(_SC_PAGESIZE);
    block.size = (size + (pagesize - 1)) & ~(pagesize - 1);
    block.start = block.exec = NULL;
    block.fd = -1;

    osl::Security aSecurity;
    rtl::OUString strDirectory;
    rtl::OUString strURLDirectory;
    if (aSecurity.getHomeDir(strURLDirectory))
        osl::File::getSystemPathFromFileURL(strURLDirectory, strDirectory);

    for (int i = strDirectory.getLength() == 0 ? 1 : 0; i < 2; ++i)
    {
        if (!strDirectory.getLength())
            strDirectory = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/tmp" ));

        strDirectory += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/.execoooXXXXXX" ));
        rtl::OString aTmpName = rtl::OUStringToOString(strDirectory, osl_getThreadTextEncoding());
        char *tmpfname = new char[aTmpName.getLength()+1];
        strncpy(tmpfname, aTmpName.getStr(), aTmpName.getLength()+1);
        if ((block.fd = mkstemp(tmpfname)) == -1)
            fprintf(stderr, "mkstemp(\"%s\") failed: %s\n", tmpfname, strerror(errno));
        if (block.fd == -1)
        {
            delete[] tmpfname;
            break;
        }
        unlink(tmpfname);
        delete[] tmpfname;
        if (ftruncate(block.fd, block.size) == -1)
        {
            perror("truncation of executable memory area failed");
            close(block.fd);
            block.fd = -1;
            break;
        }
        block.start = mmap(NULL, block.size, PROT_READ | PROT_WRITE, MAP_SHARED, block.fd, 0);
        if (block.start== MAP_FAILED) {
            block.start = 0;
        }
        block.exec = mmap(NULL, block.size, PROT_READ | PROT_EXEC, MAP_SHARED, block.fd, 0);
        if (block.exec == MAP_FAILED) {
           block.exec = 0;
        }

        //All good
        if (block.start && block.exec && block.fd != -1)
            break;

        freeBlock(block);

        strDirectory = rtl::OUString();
    }
    if (!block.start || !block.exec || block.fd == -1)
    {
       //Fall back to non-doublemmaped allocation
       block.fd = -1;
       block.start = block.exec = rtl_arena_alloc(m_arena, &block.size);
    }
    return (block.start != 0 && block.exec != 0);
}

void VtableFactory::freeBlock(Block const & block) const {
    //if the double-map failed we were allocated on the arena
    if (block.fd == -1 && block.start == block.exec && block.start != NULL)
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
    return block.start != 0;
}

void VtableFactory::freeBlock(Block const & block) const {
    rtl_arena_free(m_arena, block.start, block.size);
}
#endif

void VtableFactory::createVtables(
    GuardedBlocks & blocks, BaseOffset const & baseOffset,
    typelib_InterfaceTypeDescription * type, bool includePrimary) const
{
    if (includePrimary) {
        sal_Int32 slotCount
            = bridges::cpp_uno::shared::getPrimaryFunctions(type);
        Block block;
        if (!createBlock(block, slotCount)) {
            throw std::bad_alloc();
        }
        try {
            Slot * slots = initializeBlock(block.start, slotCount);
            unsigned char * codeBegin =
                reinterpret_cast< unsigned char * >(slots);
            unsigned char * code = codeBegin;
            sal_Int32 vtableOffset = blocks.size() * sizeof (Slot *);
            for (typelib_InterfaceTypeDescription const * type2 = type;
                 type2 != 0; type2 = type2->pBaseTypeDescription)
            {
                code = addLocalFunctions(
                    &slots, code,
#ifdef USE_DOUBLE_MMAP
                    sal_IntPtr(block.exec) - sal_IntPtr(block.start),
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
            ::std::swap(block.start, block.exec);
#endif
            blocks.push_back(block);
        } catch (...) {
            freeBlock(block);
            throw;
        }
    }
    for (sal_Int32 i = 0; i < type->nBaseTypes; ++i) {
        createVtables(blocks, baseOffset, type->ppBaseTypes[i], i != 0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
