/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_ITEM_TRANSFORMANCHOR_HXX
#define INCLUDED_SVX_ITEM_TRANSFORMANCHOR_HXX

#include <item/simple/CntInt16.hxx>
#include <svx/swframetypes.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class SVX_DLLPUBLIC TransformAnchor : public CntInt16
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock();
        virtual ItemControlBlock& GetItemControlBlock() const override;

    protected:
        TransformAnchor(RndStdIds nValue = RndStdIds::UNKNOWN);

    public:
        static std::shared_ptr<const TransformAnchor> Create(RndStdIds nValue);
        RndStdIds GetAnchorType() const { return static_cast<RndStdIds>(GetValue()); }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_SVX_ITEM_TRANSFORMANCHOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
