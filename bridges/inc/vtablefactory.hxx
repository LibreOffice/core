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

#ifndef INCLUDED_BRIDGES_INC_VTABLEFACTORY_HXX
#define INCLUDED_BRIDGES_INC_VTABLEFACTORY_HXX

#include "osl/mutex.hxx"
#include "rtl/alloc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"

#include <unordered_map>

/*See: http://people.redhat.com/drepper/selinux-mem.html*/
#if defined(LINUX) || defined(OPENBSD) || defined(FREEBSD) \
    || defined(NETBSD) || defined(MACOSX) || defined(DRAGONFLY) || defined (ANDROID)
#define USE_DOUBLE_MMAP
#endif

namespace bridges { namespace cpp_uno { namespace shared {

/** Hand out vtable structures for interface type descriptions.
 */
class VtableFactory {
public:
    // This structure is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** A vtable slot.
     */
    struct Slot;

    /** A raw vtable block.
     */
    struct Block {
        /** The start of the raw vtable block.

            It points to the start of the allocated memory block, whereas the
            vtable pointer typically points some bytes into the block (e.g.,
            skipping an RTTI pointer, see mapBlockToVtable).  Also, the block
            contains any generated code snippets, after the vtable itself.
         */
        void * start;

#ifdef USE_DOUBLE_MMAP
    /** When separately mmapping the block for writing and executing
            exec points to the same memory as start, except start is used
            exclusively for writing and exec for executing
         */
        void * exec;

    /** File handle for the underlying anonymous file
         */
        int fd;
#endif

        /** The size of the raw vtable block, in bytes.
         */
        sal_Size size;
    };

    /** The vtable structure corresponding to an interface type.
     */
    struct Vtables {
        /** The number of blocks/vtables.
         */
        sal_Int32 count;

        /** An array of blocks, representing the multiple vtables of a
            (multiple-inheritance) type.

            <p>A block is a raw vtable.  It points to the start of the allocated
            memory block, whereas the vtable pointer typically points some bytes
            into the block (e.g., skipping an RTTI pointer, see
            mapBlockToVtable).  Also, the block contains any generated code
            snippets, after the vtable itself.</p>
         */
        Block * blocks;
        Vtables()
            : count(0)
            , blocks(nullptr)
        {
        }
    };

    VtableFactory();

    ~VtableFactory();

    /** Given an interface type description, return its corresponding vtable
        structure.
     */
    Vtables getVtables(typelib_InterfaceTypeDescription * type);

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** Given a pointer to a block, turn it into a vtable pointer.
     */
    static Slot * mapBlockToVtable(void * block);

private:
    class GuardedBlocks;
    friend class GuardedBlocks;

    class BaseOffset;

    VtableFactory(VtableFactory &) = delete;
    void operator =(const VtableFactory&) = delete;

    bool createBlock(Block &block, sal_Int32 slotCount) const;

    void freeBlock(Block const & block) const;

    sal_Int32 createVtables(
        GuardedBlocks & blocks, BaseOffset const & baseOffset,
        typelib_InterfaceTypeDescription * type, sal_Int32 vtableNumber,
        typelib_InterfaceTypeDescription * mostDerived, bool includePrimary)
        const;

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** Calculate the size of a raw vtable block.

        @param slotCount  the number of virtual function slots the returned
        vtable block shall support (if there are any platform-specific slots,
        like an RTTI pointer, or a pointer to a destructor, they are not covered
        by slotCount)
        @return  the size of the raw vtable block, in bytes
     */
    static std::size_t getBlockSize(sal_Int32 slotCount);

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** Initialize a raw vtable block.

        @param block  the start address of the raw vtable block
        @param slotCount  the number of slots
        @param vtableNumber  zero-based count across all the most derived type's
        vtables (for vtable's "offset to top" slot)
        @param type  non-null most derived type (for vtable's "typeinfo pointer"
        slot)
        @return  a pointer past the last vtable slot
     */
    static Slot * initializeBlock(
        void * block, sal_Int32 slotCount, sal_Int32 vtableNumber,
        typelib_InterfaceTypeDescription * type);

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** Fill the vtable slots corresponding to all local (i.e., not inherited)
        functions of a given interface type (and generate any necessary code
        snippets for them).

        @param slots  on input, points past the vtable slot to be filled with
        the last virtual function local to the given type; on output, points to
        the vtable slot filled with the first virtual function local to the
        given type
        @param code  points to the start of the area where code snippets can be
        generated
        @param writetoexecdiff when the same code area is mapped twice, once for
        writing for code-generation, and once for code-execution, then this
        records the offset from a writable address to its executable address
        @param type  the interface type description for which to generate vtable
        slots
        @param functionOffset  the function offset of the first vtable slot
        (typically coded into the code snippet for that vtable slot)
        @param functionCount  the number of vtable slots to fill (the number of
        local functions of the given type, passed in so that it need not be
        recomputed)
        @param vtableOffset  the offset of this vtable (needed to adjust the
        this pointer, typically coded into the code snippets for all the filled
        vtable slots)
        @return  a pointer to the remaining code snippet area
     */
    static unsigned char * addLocalFunctions(
        Slot ** slots, unsigned char * code,
#ifdef USE_DOUBLE_MMAP
        sal_PtrDiff writetoexecdiff,
#endif
        typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
        sal_Int32 functionCount, sal_Int32 vtableOffset);

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** Flush all the generated code snippets of a vtable, on platforms that
        require it.

        @param begin  points to the start of the code snippet area
        @param end  points behind the end of the code snippet area
     */
    static void flushCode(
        unsigned char const * begin, unsigned char const * end);

    typedef std::unordered_map< OUString, Vtables, OUStringHash > Map;

    osl::Mutex m_mutex;
    Map m_map;

    rtl_arena_type * m_arena;
};

} } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
