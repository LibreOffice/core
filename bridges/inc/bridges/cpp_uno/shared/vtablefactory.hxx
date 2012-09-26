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

#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_VTABLEFACTORY_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_VTABLEFACTORY_HXX

#include "osl/mutex.hxx"
#include "rtl/alloc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"

#include <boost/unordered_map.hpp>

/*See: http://people.redhat.com/drepper/selinux-mem.html*/
#if defined(LINUX) || defined(OPENBSD) || defined(FREEBSD) \
    || defined(NETBSD) || defined(DRAGONFLY) || defined (ANDROID) \
    || (defined(MACOSX) && defined(__LP64__))
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
    /** When seperately mmapping the block for writing and executing
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

    VtableFactory(VtableFactory &); // not implemented
    void operator =(VtableFactory); // not implemented

    bool createBlock(Block &block, sal_Int32 slotCount) const;

    void freeBlock(Block const & block) const;

    void createVtables(
        GuardedBlocks & blocks, BaseOffset const & baseOffset,
        typelib_InterfaceTypeDescription * type, bool includePrimary) const;

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** Calculate the size of a raw vtable block.

        @param slotCount  the number of virtual function slots the returned
        vtable block shall support (if there are any platform-specific slots,
        like an RTTI pointer, or a pointer to a destructor, they are not covered
        by slotCount)
        @return  the size of the raw vtable block, in bytes
     */
    static sal_Size getBlockSize(sal_Int32 slotCount);

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** Initialize a raw vtable block.

        @param block  the start address of the raw vtable block
        @param slotCount  the number of slots
        @return  a pointer past the last vtable slot
     */
    static Slot * initializeBlock(void * block, sal_Int32 slotCount);

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
        @param writetoexecdiff when the same code area is mmaped twice, once for
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

    typedef boost::unordered_map< rtl::OUString, Vtables, rtl::OUStringHash > Map;

    osl::Mutex m_mutex;
    Map m_map;

    rtl_arena_type * m_arena;
};

} } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
