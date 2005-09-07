/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vtablefactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:10:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_VTABLEFACTORY_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_VTABLEFACTORY_HXX

#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"

#include <hash_map>

namespace bridges { namespace cpp_uno { namespace shared {

/** Hand out vtable structures for interface type descriptions.
 */
class VtableFactory {
public:
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
        char ** blocks;
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
    static void ** mapBlockToVtable(char * block);

private:
    class GuardedBlocks;
    class BaseOffset;

    VtableFactory(VtableFactory &); // not implemented
    void operator =(VtableFactory); // not implemented

    static void createVtables(
        GuardedBlocks & blocks, BaseOffset const & baseOffset,
        typelib_InterfaceTypeDescription * type, bool includePrimary);

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** Create a raw vtable block.

        @param slotCount  the number of virtual function slots the returned
        vtable block shall support (if there are any platform-specific slots,
        like an RTTI pointer, or a pointer to a destructor, they are not covered
        by slotCount)
        @param slots  output parameter returning a pointer to the first virtual
        function slot (minus any platform-specific ones, like a pointer to a
        destructor) within the returned vtable block
        @return  the start address of the raw vtable block
     */
    static char * createBlock(sal_Int32 slotCount, void *** slots);

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    /** Fill the vtable slots corresponding to all local (i.e., not inherited)
        functions of a given interface type (and generate any necessary code
        snippets for them).

        @param slots  points to the first vtable slot to be filled with the
        first virtual function local to the given type
        @param code  points to the start of the area where code snippets can be
        generated
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
        void ** slots, unsigned char * code,
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

    typedef std::hash_map< rtl::OUString, Vtables, rtl::OUStringHash > Map;

    osl::Mutex m_mutex;
    Map m_map;
};

} } }

#endif
