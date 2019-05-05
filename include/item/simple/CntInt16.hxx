/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ITEM_SIMPLE_CNTINT16_HXX
#define INCLUDED_ITEM_SIMPLE_CNTINT16_HXX

#include <item/base/ItemBase.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for SfxInt16Item
    // It uses IAdministrator_set to provide an Administrator using
    // a sorted list for fast accesses. This requires ::operator< to be
    // implemented.
    class ITEM_DLLPUBLIC CntInt16 : public ItemBase
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock();

    private:
        sal_Int16 m_nValue;

    protected:
        // constructor for derived classes that *have* to hand
        // in the to-be-used ItemControlBlock
        CntInt16(ItemControlBlock& rItemControlBlock, sal_Int16 nVal = 0);

    public:
        CntInt16(sal_Int16 nVal = 0);

        virtual bool operator==(const ItemBase&) const;
        virtual std::unique_ptr<ItemBase> clone() const;

        sal_Int16 getValue() const { return m_nValue; }
        void putValue(sal_Int16 nNew) { m_nValue = nNew; }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_SIMPLE_CNTINT16_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
