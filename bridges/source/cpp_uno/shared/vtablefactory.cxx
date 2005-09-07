/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vtablefactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:35:41 $
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

#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "guardedarray.hxx"

#include "bridges/cpp_uno/shared/vtables.hxx"

#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"

#include <hash_map>
#include <vector>

using bridges::cpp_uno::shared::VtableFactory;

class VtableFactory::GuardedBlocks: public std::vector< char * > {
public:
    GuardedBlocks(): m_guarded(true) {}

    ~GuardedBlocks();

    void unguard() { m_guarded = false; }

private:
    GuardedBlocks(GuardedBlocks &); // not implemented
    void operator =(GuardedBlocks); // not implemented

    bool m_guarded;
};

VtableFactory::GuardedBlocks::~GuardedBlocks() {
    if (m_guarded) {
        for (iterator i(begin()); i != end(); ++i) {
            delete[] *i;
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

    typedef std::hash_map< rtl::OUString, sal_Int32, rtl::OUStringHash > Map;

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

VtableFactory::VtableFactory() {}

VtableFactory::~VtableFactory() {
    osl::MutexGuard guard(m_mutex);
    for (Map::iterator i(m_map.begin()); i != m_map.end(); ++i) {
        for (sal_Int32 j = 0; j < i->second.count; ++j) {
            delete[] i->second.blocks[j];
        }
        delete[] i->second.blocks;
    }
}

VtableFactory::Vtables VtableFactory::getVtables(
    typelib_InterfaceTypeDescription * type)
{
    rtl::OUString name(type->aBase.pTypeName);
    osl::MutexGuard guard(m_mutex);
    Map::iterator i(m_map.find(name));
    if (i == m_map.end()) {
        GuardedBlocks blocks;
        createVtables(blocks, BaseOffset(type), type, true);
        Vtables vtables;
        OSL_ASSERT(blocks.size() <= SAL_MAX_INT32);
        vtables.count = static_cast< sal_Int32 >(blocks.size());
        bridges::cpp_uno::shared::GuardedArray< char * > guardedBlocks(
            new char *[vtables.count]);
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

void VtableFactory::createVtables(
    GuardedBlocks & blocks, BaseOffset const & baseOffset,
    typelib_InterfaceTypeDescription * type, bool includePrimary)
{
    if (includePrimary) {
        sal_Int32 slotCount
            = bridges::cpp_uno::shared::getPrimaryFunctions(type);
        void ** slots;
        bridges::cpp_uno::shared::GuardedArray< char > block(
            createBlock(slotCount, &slots));
        slots += slotCount;
        unsigned char * codeBegin = reinterpret_cast< unsigned char * >(slots);
        unsigned char * code = codeBegin;
        sal_Int32 vtableOffset = blocks.size() * sizeof (void **);
        for (typelib_InterfaceTypeDescription const * type2 = type; type2 != 0;
             type2 = type2->pBaseTypeDescription)
        {
            sal_Int32 functionCount
                = bridges::cpp_uno::shared::getLocalFunctions(type2);
            slots -= functionCount;
            code = addLocalFunctions(
                slots, code, type2,
                baseOffset.getFunctionOffset(type2->aBase.pTypeName),
                functionCount, vtableOffset);
        }
        flushCode(codeBegin, code);
        blocks.push_back(block.get());
        block.release();
    }
    for (sal_Int32 i = 0; i < type->nBaseTypes; ++i) {
        createVtables(blocks, baseOffset, type->ppBaseTypes[i], i != 0);
    }
}
