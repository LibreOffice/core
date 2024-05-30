/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <typelib/typedescription.hxx>
#include <vtablefactory.hxx>

using bridges::cpp_uno::shared::VtableFactory;

struct VtableFactory::Slot
{
};

VtableFactory::Slot* VtableFactory::mapBlockToVtable(void* block)
{
    return static_cast<Slot*>(block) + 2;
}

std::size_t VtableFactory::getBlockSize(sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof(Slot);
}

VtableFactory::Slot* VtableFactory::initializeBlock(void* block, sal_Int32 slotCount, sal_Int32,
                                                    typelib_InterfaceTypeDescription*)
{
    Slot* slots = mapBlockToVtable(block);
    return slots + slotCount;
}

unsigned char* VtableFactory::addLocalFunctions(Slot**, unsigned char*,
                                                typelib_InterfaceTypeDescription const*, sal_Int32,
                                                sal_Int32, sal_Int32)
{
    std::abort();
}

void VtableFactory::flushCode(unsigned char const*, unsigned char const*) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
